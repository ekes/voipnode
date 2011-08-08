<?php

/**
 * @file
 *   voipnode administration pages and alterations.
 */

/**
 * Menu callback; admin settings form.
 */
function voipnode_admin_settings() {
  drupal_add_js(drupal_get_path('module', 'voipnode') . '/voipnode.admin.js');
  $form = array();
  $form['voipnode_valid_script_names'] = array(
    '#title' => t('Valid scripts'),
    '#type' => 'checkboxes',
    '#options' => VoipScript::getScriptNames(),
    '#default_value' => variable_get('voipnode_valid_script_names', array()),
    '#description' => t('Scripts that will be available for VoIP nodes. You can select multiple scripts.'),
  );
  $form['voipnode_default_script_name'] = array(
    '#title' => t('Default script'),
    '#type' => 'select',
    '#options' => VoipScript::getScriptNames(),
    '#default_value' => variable_get('voipnode_default_script_name', ''),
    '#description' => t('The script, of those valid, that will be used by default.'),
  );
  $form['voipnode_default_extension_status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Listed'),
    '#description' => t('The default setting for if the extension is listed in a directory.'),
    '#default_value' => variable_get('voipnode_default_extension_status', 0),
    '#return_value' => 1,
  );
  $form['array_filter'] = array('#type' => 'hidden');
  $form = system_settings_form($form);
  $form['#submit'][] = 'voipnode_admin_settings_submit';
  return $form;
}

/**
 * voipnode_admin_settings() form additional submit handler.
 *
 * $form['array_filter'] = array('#type' => 'hidden'); uses numeric keys
 * the id as key and value makes life a lot easier.
 */
function voipnode_admin_settings_submit(&$form, &$form_state) {
  $valid = array();
  foreach($form_state['values']['voipnode_valid_script_names'] as $id => $value) {
    if ($value && isset($form['voipnode_valid_script_names'][$id])) { $valid[$id] = $id; }
  }
  variable_set('voipnode_valid_script_names', $valid);
}

/**
 * Menu callback; confirmation form for reindexing extensions to nodes by content-type.
 */
function voipnode_admin_reindex_confirm_form($form_state) {
  $types = node_get_types();
  $options = array();
  foreach ($types as $node_type => $type_details) {
    if (variable_get('voipnode_use_' . $node_type, FALSE)) {
      $options[$node_type] = $type_details->name;
    }
  }
  if (count($options)) {
    // only offer the form if there are some content-types used for extensions.
    $form = array();
    $form['content_type'] = array(
      '#type' => 'select',
      '#options' => $options,
      '#title' => t('Content type to reindex'),
    );
    return confirm_form($form, t('Reindex nodes with extensions'), '/admin/settings/voipnode', t('This adds extensions to any nodes in the content type that do no not have them yet, and resets all nodes to the content-type defaults.'));
  }
  else {
    return array('error' => array('#value' => t('There are no content types enabled as using VoIP extensions.')));
  }
}

/**
 * @see voipnode_admin_reindex_confirm_form().
 */
function voipnode_admin_reindex_confirm_form_submit($form, &$form_state) {
  voipnode_admin_reindex_batch($form_state['values']['content_type']);
  $form_state['redirect'] = 'admin/settings/voipnode';
}

/**
 * Set and update extensions for all nodes in the type.
 *
 * There isn't really any way round going through each node in turn, so batching the job.
 */
function voipnode_admin_reindex_batch($type) {
  $batch = array(
    'title' => t('Resetting extensions for %type', array('%type' => $type)),
    'operations' => array(
      array('_voipnode_admin_reindex_batch', array($type)),
    ),
    'finished' => '_voipnode_admin_reindex_batch_finished',
    'file' => __FILE__,
  );
  batch_set($batch);
}

/**
 * Batch operation callback.
 *
 * Calls up the next (100) nodes of the $content_type, sets defaults and calls our nodeapi update.
 * This will set defaults on all nodes of the content type, and add extensions to any that
 * don't have one yet.
 *
 * @todo check the batch counting logic.
 *
 * @see voipnode_admin_reindex_batch().
 */
function _voipnode_admin_reindex_batch($content_type, &$context) {
  if (empty($context['sandbox'])) {
    // Initiate multistep processing.
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = db_result(db_query("SELECT COUNT(DISTINCT nid) FROM {node} WHERE type = '%s'", $content_type));
  }
  $limit = 100;
  $result = db_query_range("SELECT nid FROM {node} WHERE nid > %d AND type = '%s' ORDER BY nid ASC", $context['sandbox']['current_node'], $content_type, 0, $limit);
  while ($row = db_fetch_array($result)) {
    $node = node_load($row['nid'], NULL, TRUE);
    $node->voipextension_listed = variable_get('voipnode_default_extension_status_'. $content_type, variable_get('voipnode_default_extension_status', 0));
    $node->voipextension_script = variable_get('voipnode_default_script_name_'. $content_type, variable_get('voipnode_default_script_name',''));
    // we only need our part of the hook_nodeapi
    voipnode_nodeapi($node, 'update');
    $context['sandbox']['progress']++;
    $context['sandbox']['current_node'] = $row['nid'];
  }

  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Batch finished callback.
 *
 * @see voipnode_admin_reindex_batch().
 */
function _voipnode_admin_reindex_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('The extensions have been reindexed'));
  }
  else {
    drupal_set_message(t('There was an error reindexing the extensions'), 'error');
  }
}

/**
 * Alter node type settings form.
 *
 * Add voip extension option to content type admin form.
 * @see _voipnode_node_type_form_submit().
 */
function _voipnode_form_node_type_form_alter(&$form, &$form_state) {
  $voipnode_weight = 33; // default weight of the fieldset.
  $content_type = $form['#node_type']->type;
  if (isset($form['submit']['#weight'])) {
    $voipnode_weight = $form['submit']['#weight'] - 0.99;
  }

  $form['voipnode'] = array(
    '#type' => 'fieldset',
    '#title' => t('VoIP Node'),
    '#group' => 'additional_settings',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => $voipnode_weight
  );

  $form['voipnode']['voipnode_use'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use this content type as a VoIP Node'),
    '#default_value' => variable_get('voipnode_use_'. $content_type, FALSE),
    '#attributes' => array(
      'onclick' => 'javascript: if (jQuery(this).attr("checked")) {
          jQuery("fieldset#node_type_form_voipnode_settings_id").show("slow");
          jQuery("fieldset#node_type_form_voip_directory_configuration_id").show("slow");
        }
        else {
          jQuery("fieldset#node_type_form_voipnode_settings_id").hide("fast");
          jQuery("fieldset#node_type_form_voip_directory_configuration_id").hide("fast");
        };'
    )
  );

  $voipnode_settings_style = 'display:none;';
  if (variable_get('voipnode_use_'. $content_type, FALSE)) {
    $voipnode_settings_style = 'display:block;';
  }
    // VoIP Extension field settings.
  $form['voipnode']['voipnode_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('VoIP Node Default Settings'),
    '#group' => 'additional_settings',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#attributes' => array(
      'style' => $voipnode_settings_style,
      'id' => 'node_type_form_voipnode_settings_id'
    ),
  );

  $default_script_name = variable_get('voipnode_default_script_name_'. $content_type, variable_get('voipnode_default_script_name',''));
  $form['voipnode']['voipnode_settings']['voipnode_default_script_name'] = array(
    '#type' => 'select',
    '#title' => t('Default Script Name'),
    '#options' => variable_get('voipnode_valid_script_names', VoipScript::getScriptNames()),
    '#default_value' => $default_script_name,
  );

  $default_extension_status = variable_get('voipnode_default_extension_status_'. $content_type, variable_get('voipnode_default_extension_status', 0));
  $form['voipnode']['voipnode_settings']['voipnode_default_extension_status'] = array(
    '#type' => 'radios',
    '#title' => t('Default Extension status'),
    '#options' => array(1 => t('Listed in extension directories'), 0 => t('Not listed')),
    '#default_value' => $default_extension_status ? $default_extension_status : 0
  );

  if (variable_get('voipnode_use_'. $content_type, FALSE)) {
    $form['voipnode']['voipnode_fields_configuration'] = array(
      '#type' => 'fieldset',
      '#title' => t('VoIP Directory Fields Configuration'),
      '#group' => 'additional_settings',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#attributes' => array(
        'id' => 'node_type_form_voip_directory_configuration_id',
      ),
    );

    $voipnode_configuration_options = '<div class="voipnode-'. $content_type .'-configuration-options">';
//    $voipnode_configuration_options .= l(t('Configure Extension listed in phone directory'), 'admin/content/node-type/'. $content_type .'/fields/'. VOIPEXTENSION_EXTENSION_LISTED);
//    $voipnode_configuration_options .= '<br />';
    $voipnode_configuration_options .= l(t('Configure Audio name Field'), 'admin/content/node-type/'. $content_type .'/fields/field_voipnode_aname');
    $voipnode_configuration_options .= '<br />';
    $voipnode_configuration_options .= l(t('Configure Audio Description Field'), 'admin/content/node-type/'. $content_type .'/fields/field_voipnode_adesc');
    $voipnode_configuration_options .= '<br />';
    $voipnode_configuration_options .= l(t('Configure Audio Greeting'), 'admin/content/node-type/'. $content_type .'/fields/fiold_voipnode_agreet');
    $voipnode_configuration_options .= '<br />';
    $voipnode_configuration_options .= '</div>';

    $form['voipnode']['voipnode_fields_configuration']['configuration_options'] = array(
      '#type' => 'markup',
      '#value' => $voipnode_configuration_options
    );
  }
  $form['#submit'][] = 'voipnode_form_node_type_form_submit';
}

/**
 * Submit callback for _voipnode_node_type_form_submit().
 */
function _voipnode_form_node_type_form_submit($form, &$form_state) {
  module_load_include('inc', 'voipnode', 'voipnode.cck_fields');

  // VoIP Extension field doesn't exists yet.
  if (!isset($form_state['values']['voipnode_use'])) {
    return TRUE;
  }

  $content_type = $form_state['values']['type'];
  if ($form_state['values']['voipnode_use']) {
    // Enable voipnode on type. add fields and group.
    // Adding extensions to existing nodes is an admin task. 
    // Using the admin task existing disabled extensions can all be defaulted to enabled.
    _voipnode_change_audio_fields_status($content_type, 1);
    $group = _voipnode_extension_group_defintion();
    fieldgroup_save_group($content_type, $group);
    _voipnode_create_audio_fields($content_type);
    $group_name = 'group_voipnode_settings';
    $fields = array(
      'field_voipnode_aname',
      'field_voipnode_adesc',
      'field_voipnode_agreet',
    );
    // Assign the voip extension fields to the selcted field group.
    foreach ($fields as $index => $field_name) {
      $form_values = array();
      $form_values['type_name']  = $content_type;
      $form_values['group']      = $group_name;
      $form_values['field_name'] = $field_name;
      fieldgroup_update_fields($form_values);
    }

    drupal_set_message(t('VoIP Extension field have been associated with %content_type', array('%content_type' => $content_type)));
  }
  else {
    // Disable voipnode on type. Hide the audio fields, remove the fieldset, disable the extensions.
    _voipnode_change_audio_fields_status($content_type, 0);
    fieldgroup_delete($content_type, 'group_voipnode_settings');
    voipextension_disable_module_extensions('voipnode', $content_type);
    drupal_set_message(t('VoIP Extensions have been disabled for %content_type', array('%content_type' => $content_type)));
  }
}
