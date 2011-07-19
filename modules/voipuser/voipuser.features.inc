<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function voipuser_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_node_info().
 */
function voipuser_node_info() {
  $items = array(
    'voipuser' => array(
      'name' => t('VoIP user extension'),
      'module' => 'features',
      'description' => t('Provides users with an extension node.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'has_body' => '1',
      'body_label' => t('Body'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}

/**
 * Implementation of hook_views_api().
 */
function voipuser_views_api() {
  return array(
    'api' => '2',
  );
}
