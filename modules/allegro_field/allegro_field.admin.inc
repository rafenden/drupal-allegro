<?php

/**
 * @file
 * Allegro Field administration pages.
 */

/**
 * Settings page.
 */
function allegro_field_settings_form() {
  if (!allegro_is_configured_correctly()) {
    drupal_set_message(t('Allegro is not properly configured. Plese go to <a href="@config-url">configuration page</a> and set up correct login creditials.', array('@config-url' => url('admin/allegro/settings'))), 'error');
    return array();
  }

  $form = array();
  $form['update'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update data'),
  );
  $form['update']['download_fields'] = array(
    '#type' => 'submit',
    '#value' => t('Update fields'),
    '#submit' => array('allegro_field_download_batch'),
  );
  return $form;
}

/**
 * Menu callback: run a batch downloading Allegro fields.
 */
function allegro_field_download_batch() {
  $transaction = db_transaction();
  try {
    $batch = array(
      'title' => t('Downloading fields'),
      'finished' => '_allegro_field_download_batch_finished',
      'file' => drupal_get_path('module', 'allegro_field') . '/allegro_field.admin.inc',
    );

    $limit = 1000;
    $count = 20000;
    $iterations = $count / $limit;

    allegro_field_delete_by_country(variable_get('allegro_country_code'));

    for ($i = 0; $i <= $iterations; $i++) {
      $batch['operations'][] = array('allegro_field_download', array($i, $limit));
    }

    batch_set($batch);
    batch_process();
  }
  catch (SoapFault $e) {
    $transaction->rollback();
    drupal_set_message($e->faultstring, 'error');
    allegro_watchdog_exception('allegro_field', $e, $e->faultactor);
  }
}

/**
 * Performs post-processing for allegro_field_download_batch().
 *
 * @param bool $success
 *   A boolean indicating whether the re-build process has completed.
 * @param array $results
 *   An array of results information.
 * @param array $operations
 *   An array of function calls (not used in this function).
 */
function _allegro_field_download_batch_finished($success, $results = array(), $operations = array()) {
  if ($success) {
    drupal_set_message(t('The fields have been downloaded.'));
    watchdog('allegro_field', 'Fields was updated.');
    variable_set('allegro_field_last_update', time());
  }
  else {
    drupal_set_message(t('The fields have not been properly downloaded.'), 'error');
    watchdog('allegro_field', 'The fields have not been properly downloaded.');
  }
}