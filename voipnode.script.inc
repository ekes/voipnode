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


/**
 * Basic script to read a voipnode directory
 *
 * @return
// TODO: should not return anything, but execute the script associated w/ extension. Actually, should return the extension chosen. Should also have an 'no_run' option to not run the script...
 * Sets script variable with the extension number selected, or '' in case of 
 * of timeout. 
 */
function _voipnode_get_directory_script($extension_type) {
//TODO: check for invalid extension numbers, too!
//TODO: add prompts to an $options argument
// TODO: add module_type param,move this function to voipextension

  $script = new VoipScript('voipnode_directory');

  $rx = voipextension_load_module_extensions('voipnode', $extension_type);

  if(!$rx) {
    $p_no_extensions = t('This directory is currently empty. ');
    $script->addSay($p_no_extensions);
  }
  else {
    $p_instructions = t("Extension's directory. At any time, press the desired extension number or press the pound key to go back. ");
    $p_no_input = t('No input received. Please try again. ');
    $p_invalid_extension = t('Invalid extension number. Please try again. ');

    $prompt = $p_instructions;

    foreach($rx as $x) {
      $title = voipextension_get_title($x);
      $number = $x['eid'];
      $x_numbers[] = $number;
// TODO: sort based on $node->title? add directory_title to voipextension?
// TODO: enable multiple types in load_extensions
// TODO: remove id from  locad_extensions result index
      $prompt .= t('for @title, press @number. ',
                   array('@title' => $title, '@number' => $number));
    }

    $script->addSet('__dir_counter', 0);
    $script->addSet('extension_selected', NULL);
    $s_x_numbers = serialize($x_numbers);

    $script->addLabel('read_directory');
    $script->addGetInput($prompt, 1, '', 3);
    $script->addGotoIf('no_input', '^is_null(%input_digits)');
    $script->addGotoIf('go_back', '^%input_digits == "#"');
    $script->addGotoIf('invalid_extension',
                       "^!in_array(%input_digits, unserialize($s_x_numbers))");
//TODO: for chosen extensions, should the system automatically execute the associated script? Probably yes...
    $script->addSet('chosen_extension', '%input_digits');
    $script->addGoto('go_back');

    $script->addLabel('no_input');
    $script->addSet('__dir_counter', '^%__dir_counter + 1');
    $script->addGotoIf('go_back', "^%__dir_counter >= 3");
    $script->addSay($p_no_input);
    $script->addGoto('read_directory');

    $script->addLabel('invalid_extension');
    $script->addSet('__dir_counter', '^%__dir_counter + 1');
    $script->addGotoIf('go_back', "^%__dir_counter >= 3");
    $script->addSay($p_invalid_extension);
    $script->addGoto('read_directory');

    $script->addLabel('go_back');
    $script->addUnset('__dir_counter');
    
  }

  return $script;
}
