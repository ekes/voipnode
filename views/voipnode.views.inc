<?php

/**
 * @file
 *   Adds relationship from voip nodes to the extensions table.
 */

/**
 * Implementation of hook_views_data_alter().
 */
function voipnode_views_data_alter(&$data) {
  $data['node']['voipnode'] = array(
    'group' => t('VoIP Extension'),
    'title' => t('Extension'),
    'help' => t('Relate node to VoIP extension.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'voipextension',
      'base field' => 'module_id',
      'relationship field' => 'nid',
      'label' => t('Extension'),
      'extra' => array(
        array('field' => 'module', 'value' => 'voipnode',),
      ),
    ),
  );

  $data['voipextension']['voipnode'] = array(
    'title' => t('VoIP node'),
    'help' => t('Relate to VoIP nodes.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'node',
      'base field' => 'nid',
      'relationship table' => 'voipextension',
      'relationship field' => 'module_id',
      'label' => t('VoIP Node'),
      'extra' => array(
        array('table' => 'voipextension', 'field' => 'module', 'value' => 'voipnode',)
      ),
    ),
  );
}
