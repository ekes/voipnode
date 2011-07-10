<?php

/**
 * @file
 *   voipscripts for voipnode.
 */

/**
 * Basic default script to read attached audio.
 */
function voipnode_basic_script($node) {
  $body = strip_tags($node->body);

  $script = new VoipScript('voipnode_basic');
  if (! empty($node->field_voipnode_agreet[0]['filepath'])) {
    $greeting = file_create_url(field_file_urlencode_path($node->field_voipnode_agreet[0]['filepath']));
    $script->addSay($greeting);
  }
  $script->addSay($body);
  $script->addReturn;

  return $script;
}
