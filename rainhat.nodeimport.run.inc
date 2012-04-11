<?php
/**
 * @file
 * This file demonstrates how drupal must be doing something wrong. 
 * 6 functions for a 2 page form? 
 * Drupal is where KISS and DRY go to die. 
 */

function rainhat_node_import_run_form($form, &$form_state, $node_import) {
  
  $form['#field_import'] = $field_import;

  if (!empty($form_state['page']) && $form_state['page'] == 2) {
    return rainhat_node_import_run_form_list_pids($form, $form_state);
  }
  
  if (!empty($form_state['page']) && $form_state['page'] == 3) {
    return rainhat_node_import_run_form_execute($form, $form_state);
  }

  // Otherwise we build page 1.
  $form_state['page_num'] = 1;

  $form['search'] = array(
    '#type' => 'textfield',
    '#title' => t('pid search'),
    '#size' => 40,
    '#maxlength' => 256,
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['search']) ? $form_state['values']['search'] : '',
  );
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => 'Next >>',
    '#submit' => array('rainhat_node_import_run_form_next_submit'),
  );
  return $form;
}

function rainhat_node_import_run_form_list_pids($form, &$form_state) {
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('A basic multistep form (page 2)'),
  );

  $form['color'] = array(
    '#type' => 'textfield',
    '#title' => t('Favorite color'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['color']) ? $form_state['values']['color'] : '',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('form_example_tutorial_8_page_two_submit'),
  );
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('<< Back'),
    '#submit' => array('form_example_tutorial_8_page_two_back'),
    // We won't bother validating the required 'color' field, since they
    // have to come back to this page to submit anyway.
    '#limit_validation_errors' => array(),
  );
  return $form;
}

function form_example_tutorial_8_next_submit($form, &$form_state) {

  // Values are saved for each page.
  // to carry forward to subsequent pages in the form.
  // and we tell FAPI to rebuild the form.
  $form_state['page_values'][1] = $form_state['values'];

  if (!empty($form_state['page_values'][2])) {
    $form_state['values'] = $form_state['page_values'][2];
  }

  // When form rebuilds, it will look at this to figure which page to build.
  $form_state['page_num'] = 2;
  $form_state['rebuild'] = TRUE;
}

/**
 * Back button handler submit handler.
 *
 * Since #limit_validation_errors = array() is set, values from page 2
 * will be discarded. We load the page 1 values instead.
 */
function form_example_tutorial_8_page_two_back($form, &$form_state) {
  $form_state['values'] = $form_state['page_values'][1];
  $form_state['page_num'] = 1;
  $form_state['rebuild'] = TRUE;
}

/**
 * The page 2 submit handler.
 *
 * This is the final submit handler. Gather all the data together and output
 * it in a drupal_set_message().
 */
function form_example_tutorial_8_page_two_submit($form, &$form_state) {
  // Normally, some code would go here to alter the database with the data
  // collected from the form. Instead sets a message with drupal_set_message()
  // to validate that the code worked.
  $page_one_values = $form_state['page_values'][1];
  drupal_set_message(t('The form has been submitted. name="@first @last", year of birth=@year_of_birth',
  array('@first' => $page_one_values['first'], '@last' => $page_one_values['last'], '@year_of_birth' => $page_one_values['year_of_birth'])));

  if (!empty($page_one_values['first2'])) {
    drupal_set_message(t('Second name: name="@first @last", year of birth=@year_of_birth',
    array('@first' => $page_one_values['first2'], '@last' => $page_one_values['last2'], '@year_of_birth' => $page_one_values['year_of_birth2'])));
  }
  drupal_set_message(t('And the favorite color is @color', array('@color' => $form_state['values']['color'])));

  // If we wanted to redirect on submission, set $form_state['redirect']
  // $form_state['redirect'] = 'node'; // Redirects the user to /node.
}
