<?php

/**
 * @file
 * Forms to administer the rainhat defaults.
 */

/**
 * Admin configuration form builder.
 */
function rainhat_admin_form($form, &$form_state) {

  //Fedora Connection Information
  
  $form['rainhat_fedora_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Rainhat\'s Default Fedora Connection URL'),
    '#default_value' => variable_get('rainhat_default_fedora_url'),
    '#size' => 60,
    '#maxlength' => 120,
    '#required' => TRUE,  
  );    


  $form['rainhat_fedora_admin_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => variable_get('rainhat_default_fedora_admin_username'),
    '#size' => 60,
    '#maxlength' => 120,
    '#required' => TRUE,
  );

  $form['rainhat_fedora_admin_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('rainhat_default_fedora_admin_password'),
    '#size' => 60,
    '#maxlength' => 120,
    '#required' => TRUE,
  );

  // Submit button
  $form['rainhat_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  return $form;
}

/**
 * Admin configuration form submit.
 */
function rainhat_admin_form_submit($form, &$form_state) {

  variable_set('rainhat_default_fedora_url',
    $form_state['values']['rainhat_fedora_url']);
  
  variable_set('rainhat_default_fedora_admin_username',
    $form_state['values']['rainhat_fedora_admin_username']);
    
  variable_set('rainhat_default_fedora_admin_password',
    $form_state['values']['rainhat_fedora_admin_password']);

  drupal_set_message(t('The settings have been saved'));

}
