<?php

/**
 * @file
 *   voipnode administration pages and alterations.
 */

/**
 * Admin settings form.
 *
 * @todo
 */
function voipnode_admin_settings() {
  $form['temp'] = array(
    '#value' => '<p>TODO: Offer option to select valid scripts<br />And to bulk update a content type for missing extensions.</p>',
  );
  return $form;
}

/**
 * Alter node type settings form.
 *
 * Add voip extension option to content type admin form.
 * @see _voipnode_node_type_form_submit().
 */
function _voipnode_node_type_form_alter(&$form, &$form_state, $form_id) {
  $voipnode_weight = 33; // default weight of the fieldset.
  $content_type = $form['#node_type']->type;
  if (isset($form['submit']['#weight'])) {
    $voipnode_weight = $form['submit']['#weight'] - 0.99;
  }

  $form['voipnode'] = array(
    '#type' => 'fieldset',
    '#title' => t('VoIPNode'),
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
    )
  );

  $default_script_name = variable_get('default_script_name_'. $content_type, variable_get('voipnode_script_name',''));
  $form['voipnode']['voipnode_settings']['default_script_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Script Name'),
    '#maxlength' => 100,
    '#size' => 30,
    '#default_value' => $default_script_name
  );

  $default_extension_status = variable_get('default_extension_status_'. $content_type, variable_get('voipnode_status', 0));
  $form['voipnode']['voipnode_settings']['default_extension_status'] = array(
    '#type' => 'radios',
    '#title' => t('Default Extension status'),
    '#options' => array(1 => t('Enabled'), 0 => t('Disabled')),
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
        'id' => 'node_type_form_voip_directory_configuration_id'
      )
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
  $form['#submit'][] = 'voipnode_node_type_form_submit';
}

/**
 * Submit callback for _voipnode_node_type_form_submit().
 */
function _voipnode_node_type_form_submit($form, &$form_state) {
  module_load_include('inc', 'voipnode', 'voipnode.cck_fields');

  // VoIP Extension field doesn't exists yet.
  if (!isset($form_state['values']['voipnode_use'])) {
    return TRUE;
  }

  $content_type = $form_state['values']['type'];
  if ($form_state['values']['voipnode_use']) {
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
    _voipnode_change_audio_fields_status($content_type, 0);
    fieldgroup_delete($content_type, 'group_voipnode_settings');
  }
}
