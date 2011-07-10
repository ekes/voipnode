<?php

/**
 * @file
 *   voipscripts for voipnode.
 */

/**
 * Basic default script to read attached audio.
 */
function voipnode_basic_script($node) {
  $greeting = file_create_url(field_file_urlencode_path($node->field_voipname_agreet[0]['filepath']));
  $body = $node->body;

  $script = new VoipScript('voipnode_basic');
  $script->addSay($greeting);
  $script->addSay($body);
  $script->addReturn;

  return $script;
}
