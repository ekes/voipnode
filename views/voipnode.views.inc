<?php

/**
 * @file
 *   Adds relationship from voip nodes to the extensions table.
 */

/**
 * Implementation of hook_views_data_alter().
 */
function voipnode_views_data_alter(&$data) {
  // trouble with doing this is it's then the only way to join on the node table
  // guess we need a psudo table / field
  $data['voipextension']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'module_id',
      'extra' => array(
        array('field' => 'module', 'value' => 'voipnode',),
      ),
    ),
  );

  $data['voipextension']['voipnode'] = array(
    'title' => t('voipnode'),
    'help' => t('Relate to VoIP nodes.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'node',
      'base field' => 'nid',
      'relationship table' => 'voipextension',
      'relationship field' => 'module_id',
      'label' => t('extension'),
      'extra' => array(
        array('table' => 'voipextension', 'field' => 'module', 'value' => 'voipnode',)
      ),
    ),
  );
}
