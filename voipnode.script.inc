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
  if ($greeting = voipnode_get_greeting($node)) {
    $script->addSay($greeting);
  }
  $script->addSay($body);
  $script->addReturn;

  return $script;
}
