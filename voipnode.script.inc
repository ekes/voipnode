<?php

/**
 * @file
 *   voipscripts for voipnode.
 */

/**
 * Basic default script to read voipnode contents
 */
function _voipnode_get_basic_script($node) {
//TODO: read the other elements of the node, too!

  $body = strip_tags($node->body);

  $script = new VoipScript('voipnode_basic');
  if ($greeting = voipnode_get_greeting($node)) {
    $script->addSay($greeting);
  }
  $script->addSay($body);
  $script->addReturn();

  return $script;
}

