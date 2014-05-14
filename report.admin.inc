<?php

function report_admin_page($report) {
  $output = '';
  
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Reports'), 'report'),
    l($report->title, 'report/' . $report->machine_name),
  );
  drupal_set_breadcrumb($breadcrumb);
  
  $output .= '<div>' . l(t('Edit'), 'report/' . $report->machine_name . '/admin/edit') . '</div>';
  
  $header = array(
    t('Parameter'),
    t('Type'),
    t('Category'),
    '',
    '',
  );
  $rows = array();
  $types = report_get_param_type_list();
  foreach ($report->params as $data) {
    $rows[] = array(
      $data['machine_name'],
      $types[$data['type']],
      check_plain($data['category']),
      l(t('Edit'), 'report/' . $report->machine_name . '/admin/param/' . $data['id'] . '/edit'),
      l(t('Delete'), 'report/' . $report->machine_name . '/admin/param/' . $data['id'] . '/delete'),
    );
  }
  
  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('No parameters found.'), 'colspan' => count($header)),
    );
  }
  
  $output .= '<div>' . l(t('Add parameter'), 'report/' . $report->machine_name . '/admin/param/add') . '</div>';
  $output .= theme('table', $header, $rows);
  
  $header = array(
    t('Column'),
    t('Type'),
    t('Table'),
    '',
    '',
  );
  $rows = array();
  foreach ($report->columns as $data) {
    $rows[] = array(
      $data['machine_name'],
      $types[$data['type']],
      check_plain($data['table_name']),
      l(t('Edit'), 'report/' . $report->machine_name . '/admin/param/' . $data['id'] . '/edit'),
      l(t('Delete'), 'report/' . $report->machine_name . '/admin/param/' . $data['id'] . '/delete'),
    );
  }
  
  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('No parameters found.'), 'colspan' => count($header)),
    );
  }
  
  $output .= theme('table', $header, $rows);
  
  $header = array(
    t('Condition'),
    t('Table'),
    t('Query key'),
    t('Local key'),
    t('Field name'),
    t('Expression'),
    '',
    '',
  );
  $rows = array();
  foreach ($report->conditions as $data) {
    $rows[] = array(
      $data['id'],
      $data['table_name'],
      $data['query_key'],
      $data['local_key'],
      $data['field_name'],
      $data['expression'],
      l(t('Edit'), 'report/' . $report->machine_name . '/admin/condition/' . $data['id'] . '/edit'),
      l(t('Delete'), 'report/' . $report->machine_name . '/admin/condition/' . $data['id'] . '/delete'),
    );
  }
  
  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('No conditions found.'), 'colspan' => count($header)),
    );
  }
  
  $output .= '<div>' . l(t('Add condition'), 'report/' . $report->machine_name . '/admin/condition/add') . '</div>';
  $output .= theme('table', $header, $rows);
  
  return $output;
}

function report_edit_form($form_state, $report = NULL) {
  if (empty($report->id)) {
    $report = report_init_object();
    $breadcrumb = array(
      l(t('Home'), '<front>'),
      l(t('Reports'), 'report'),
    );
    drupal_set_breadcrumb($breadcrumb);
  }
  else {
    drupal_set_title(t('Edit report <em>@name</em>', array('@name' => $report->machine_name)));
    $breadcrumb = array(
      l(t('Home'), '<front>'),
      l(t('Reports'), 'report'),
      l($report->title, 'report/' . $report->machine_name),
    );
    drupal_set_breadcrumb($breadcrumb);
  }

  $form = array();
  $form['#report'] = $report;

  $form['machine_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#required' => TRUE,
    '#default_value' => $report->machine_name,
  );

  $languages = language_list('enabled');
  foreach ($languages[1] as $lang => $data) {
    $form['title_' . $lang] = array(
      '#type' => 'textfield',
      '#title' => t('Title (@lang)', array('@lang' => $lang)),
      '#default_value' => $report->details[$lang]['title'],
    );
  }

  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#delta' => 20,
    '#default_value' => $report->weight,
  );

  $form['status'] = array(
    '#type' => 'select',
    '#title' => t('Status'),
    '#options' => report_get_status_list(),
    '#default_value' => $report->status,
  );

  $form['validation'] = array(
    '#type' => 'textarea',
    '#title' => t('Validation'),
    '#rows' => 4,
    '#default_value' => $report->validation,
  );

  $form['actions'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  if (!empty($report->id)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );

    $form['actions']['cancel'] = array(
      '#value' => l(t('Cancel'), 'report/' . $report->machine_name . '/admin'),
    );
  }

  return $form;
}

function report_edit_form_validate($form, &$form_state) {
  $report = $form['#report'];

  if (!preg_match('|^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$|', $form_state['values']['machine_name'])) {
    form_set_error('machine_name', t("Invalid name for machine name."));
  }

  $sql = "SELECT 1 FROM {reports} WHERE id <> %d AND machine_name = '%s'";
  if (db_result(db_query($sql, $report->id, $form_state['values']['machine_name']))) {
    form_set_error('machine_name', t('This machine name already exists.'));
  }
}

function report_edit_form_submit($form, &$form_state) {
  $report = $form['#report'];

  if ($form_state['values']['op'] == t('Delete')) {
    $form_state['redirect'] = 'report/' . $report->machine_name . '/admin/delete';
    return;
  }

  $report->machine_name = $form_state['values']['machine_name'];
  $report->weight = $form_state['values']['weight'];
  $report->status = $form_state['values']['status'];
  $report->validation = $form_state['values']['validation'];

  $languages = language_list('enabled');
  foreach ($languages[1] as $lang => $data) {
    $report->details[$lang]['title'] = $form_state['values']['title_' . $lang];
  }

  if (report_save($report)) {
    $attr = array(
      '@name' => $report->machine_name,
      '@id' => $report->id,
    );
    drupal_set_message(t('The report <em>@name</em> has been saved successfully', $attr));
    watchdog('report', 'Report @name (@id) saved', $attr);
    $form_state['redirect'] = array('report/' . $report->machine_name . '/admin');
  }
  else {
    $attr = array(
      '@name' => $report->machine_name,
      '@id' => $report->id,
      '@data' => print_r($report, TRUE),
    );
    drupal_set_message(t('Failed to save the report <em>@name</em>', $attr), 'error');
    watchdog('report', 'Report @name (@id) save failed. Data @data', $attr, WATCHDOG_ERROR);
  }
}

function report_delete_form($form_state, $report) {
  $form = array();
  $form['#report'] = $report;

  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Reports'), 'report'),
    l($report->title, 'report/' . $report->machine_name),
  );
  drupal_set_breadcrumb($breadcrumb);
  
  $question = t('Are you sure you want to delete the report <em>@name</em>?',
    array('@name' => $report->machine_name));
  $path = 'report/' . $report->machine_name . '/admin/edit';

  return confirm_form($form, $question, $path);
}

function report_delete_form_submit($form, &$form_state) {
  $report = $form['#report'];

  if (report_delete($report)) {
    $attr = array(
      '@name' => $report->machine_name,
      '@id' => $report->id,
    );
    drupal_set_message(t('The report <em>@name</em> has been deleted successfully', $attr));
    watchdog('report', 'Report @name (@id) deleted', $attr);
    $form_state['redirect'] = 'admin/build/report';
  }
  else {
    $attr = array(
      '@name' => $report->machine_name,
      '@id' => $report->id,
    );
    drupal_set_message(t('Failed to delete the report <em>@name</em>', $attr), 'error');
    watchdog('report', 'Report @name (@id) delete failed', $attr, WATCHDOG_ERROR);
  }
}

function report_param_edit_form($form_state, $report, $param_id = 0) {
  if ($param_id && empty($report->params[$param_id]) && empty($report->columns[$param_id])) {
    drupal_not_found();
    exit();
  }
  
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Reports'), 'report'),
    l($report->title, 'report/' . $report->machine_name),
  );
  drupal_set_breadcrumb($breadcrumb);
  
  drupal_add_js(drupal_get_path('module', 'report') . '/report.js');
  
  $languages = language_list('enabled');
  
  if (empty($param_id)) {
    $param = array(
      'report_id' => $report->id,
      'field_id' => 0,
      'param_type' => REPORT_FIELD_PARAM,
      'machine_name' => '',
      'table_name' => '',
      'type' => '',
      'weight' => 0,
      'category' => '',
      'field_size' => '',
      'required' => '',
      'default_value' => '',
      'source' => '',
      'details' => array(),
    );
    
    foreach ($languages[1] as $lang => $data) {
      $param['details'] = array(
        'param_id' => 0,
        'language' => $lang,
        'title' => '',
        'description' => '',
      );
    }
  }
  else {
    $param = empty($report->params[$param_id]) ? $report->columns[$param_id] : $report->params[$param_id];
    drupal_set_title(t('Edit parameter <em>@name</em>', array('@name' => $param['machine_name'])));
  }
  
  $form = array();
  $form['#report'] = $report;
  $form['#param'] = $param;

  $form['param_type'] = array(
    '#type' => 'select',
    '#title' => t('Type of parameter'),
    '#options' => array(
      REPORT_FIELD_PARAM => t('Parameter'),
      REPORT_FIELD_COLUMN => t('Column'),
    ),
    '#default_value' => $param['param_type'],
  );

  $form['machine_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#required' => TRUE,
    '#default_value' => $param['machine_name'],
  );

  $form['field_id'] = array(
    '#type' => 'select',
    '#title' => t('Field'),
    '#options' => _resource_get_section_fields(),
    '#default_value' => $param['field_id'],
  );
  
  $form['table_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Table name'),
    '#default_value' => $param['table_name'],
  );

  foreach ($languages[1] as $lang => $data) {
    $form['title_' . $lang] = array(
      '#type' => 'textfield',
      '#title' => t('Title (@lang)', array('@lang' => $lang)),
      '#default_value' => $param['details'][$lang]['title'],
    );
  }

  foreach ($languages[1] as $lang => $data) {
    $form['description_' . $lang] = array(
      '#type' => 'textfield',
      '#title' => t('Description (@lang)', array('@lang' => $lang)),
      '#default_value' => $param['details'][$lang]['description'],
    );
  }

  $form['category'] = array(
    '#type' => 'textfield',
    '#title' => t('Category'),
    '#default_value' => $param['category'],
  );

  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => report_get_param_type_list(),
    '#default_value' => $param['type'],
  );

  $form['required'] = array(
    '#type' => 'checkbox',
    '#title' => t('Required'),
    '#default_value' => $param['required'],
  );

  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#delta' => 20,
    '#default_value' => $param['weight'],
  );

  $form['default_value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $param['default_value'],
  );

  $form['field_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Field size'),
    '#default_value' => $param['field_size'],
  );

  $form['source'] = array(
    '#type' => 'textarea',
    '#title' => t('Source'),
    '#default_value' => $param['source'],
  );

  $form['actions'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  if (!empty($param['id'])) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );
  }

  $form['actions']['cancel'] = array(
    '#value' => l(t('Cancel'), 'report/' . $report->machine_name . '/admin'),
  );

  return $form;
}

function report_param_edit_form_validate($form, &$form_state) {
  if (!preg_match('|^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$|', $form_state['values']['machine_name'])) {
    form_set_error('machine_name', t("Invalid name for machine name."));
  }
}

function report_param_edit_form_submit($form, &$form_state) {
  $report = $form['#report'];
  $param = $form['#param'];

  if ($form_state['values']['op'] == t('Delete')) {
    $form_state['redirect'] = 'report/' . $report->machine_name . '/admin/param/' . $param['id'] . '/delete';
    return;
  }

  $param['field_id'] = $form_state['values']['field_id'];
  $param['param_type'] = $form_state['values']['param_type'];
  $param['machine_name'] = $form_state['values']['machine_name'];
  $param['table_name'] = $form_state['values']['table_name'];
  $param['type'] = $form_state['values']['type'];
  $param['weight'] = $form_state['values']['weight'];
  $param['category'] = $form_state['values']['category'];
  $param['field_size'] = $form_state['values']['field_size'];
  $param['required'] = $form_state['values']['required'];
  $param['default_value'] = $form_state['values']['default_value'];
  $param['source'] = $form_state['values']['source'];

  $languages = language_list('enabled');
  foreach ($languages[1] as $lang => $data) {
    $param['details'][$lang]['title'] = $form_state['values']['title_' . $lang];
    $param['details'][$lang]['description'] = $form_state['values']['description_' . $lang];
  }

  if (report_param_save($param)) {
    $attr = array(
      '@name' => $param['machine_name'],
      '@id' => $param['id'],
    );
    drupal_set_message(t('The param <em>@name</em> has been saved successfully', $attr));
    watchdog('report', 'Field @name (@id) saved', $attr);
    $form_state['redirect'] = array('report/' . $report->machine_name . '/admin');
  }
  else {
    $attr = array(
      '@name' => $param['machine_name'],
      '@id' => $param['id'],
      '@data' => print_r($param, TRUE),
    );
    drupal_set_message(t('Failed to save the param <em>@name</em>', $attr), 'error');
    watchdog('report', 'Field @name (@id) save failed. Data @data', $attr, WATCHDOG_ERROR);
  }
}

function report_param_delete_form($form_state, $report, $param_id) {
  if (empty($report->params[$param_id])) {
    drupal_not_found();
    exit();
  }

  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Reports'), 'report'),
    l($report->title, 'report/' . $report->machine_name),
  );
  drupal_set_breadcrumb($breadcrumb);
  
  $form = array();
  $form['#report'] = $report;
  $form['#param_id'] = $param_id;

  $question = t('Are you sure you want to delete the param <em>@name</em>?',
    array('@name' => $report->params[$param_id]['machine_name']));
  $path = 'report/' . $report->machine_name . '/admin/param/' . $param_id . '/edit';

  return confirm_form($form, $question, $path);
}

function report_param_delete_form_submit($form, &$form_state) {
  $report = $form['#report'];
  $param_id = $form['#param_id'];

  if (report_param_delete($report->params[$param_id])) {
    $attr = array(
      '@name' => $report->params[$param_id]['machine_name'],
      '@id' => $report->params[$param_id]['id'],
    );
    drupal_set_message(t('The param <em>@name</em> has been deleted successfully', $attr));
    watchdog('report', 'Field @name (@id) deleted', $attr);
    $form_state['redirect'] = 'report/' . $report->machine_name . '/admin';
  }
  else {
    $attr = array(
      '@name' => $report->params[$param_id]['machine_name'],
      '@id' => $report->params[$param_id]['id'],
    );
    drupal_set_message(t('Failed to delete the report <em>@name</em>', $attr), 'error');
    watchdog('report', 'Report @name (@id) delete failed', $attr, WATCHDOG_ERROR);
  }
}

function report_condition_edit_form($form_state, $report, $condition_id = 0) {
  if ($condition_id && empty($report->conditions[$condition_id])) {
    drupal_not_found();
    exit();
  }
  
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Reports'), 'report'),
    l($report->title, 'report/' . $report->machine_name),
  );
  drupal_set_breadcrumb($breadcrumb);
  
  if (empty($condition_id)) {
    $condition = array(
      'report_id' => $report->id,
      'table_name' => '',
      'query_key' => '',
      'local_key' => '',
      'field_name' => '',
      'expression' => '',
    );
  }
  else {
    $condition = $report->conditions[$condition_id];
    drupal_set_title(t('Edit condition <em>@name</em>', array('@name' => $condition['id'])));
  }
  
  $form = array();
  $form['#report'] = $report;
  $form['#condition'] = $condition;

  $form['table_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Table name'),
    '#required' => TRUE,
    '#default_value' => $condition['table_name'],
  );

  $form['query_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Query key'),
    '#required' => TRUE,
    '#default_value' => $condition['query_key'],
  );

  $form['local_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Local key'),
    '#required' => TRUE,
    '#default_value' => $condition['local_key'],
  );

  $form['field_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Field name'),
    '#required' => TRUE,
    '#default_value' => $condition['field_name'],
  );

  $form['expression'] = array(
    '#type' => 'textfield',
    '#title' => t('Expression'),
    '#required' => TRUE,
    '#default_value' => $condition['expression'],
  );

  $form['actions'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  if (!empty($condition['id'])) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );
  }
  
  $form['actions']['cancel'] = array(
    '#value' => l(t('Cancel'), 'report/' . $report->machine_name . '/admin'),
  );
  
  return $form;
}

function report_condition_edit_form_submit($form, &$form_state) {
  $report = $form['#report'];
  $condition = $form['#condition'];

  if ($form_state['values']['op'] == t('Delete')) {
    $form_state['redirect'] = 'report/' . $report->machine_name . '/admin/condition/' . $condition['id'] . '/delete';
    return;
  }

  $condition['table_name'] = $form_state['values']['table_name'];
  $condition['query_key'] = $form_state['values']['query_key'];
  $condition['local_key'] = $form_state['values']['local_key'];
  $condition['field_name'] = $form_state['values']['field_name'];
  $condition['expression'] = $form_state['values']['expression'];

  if (drupal_write_record('report_conditions', $condition, empty($condition['id']) ? array() : 'id') !== FALSE) {
    $attr = array(
      '@id' => $condition['id'],
    );
    drupal_set_message(t('The condition <em>@id</em> has been saved successfully', $attr));
    watchdog('report', 'Condition @id saved', $attr);
    $form_state['redirect'] = array('report/' . $report->machine_name . '/admin');
  }
  else {
    $attr = array(
      '@id' => $condition['id'],
      '@data' => print_r($condition, TRUE),
    );
    drupal_set_message(t('Failed to save the condition <em>@id</em>', $attr), 'error');
    watchdog('report', 'Field @id save failed. Data @data', $attr, WATCHDOG_ERROR);
  }
}

function report_condition_delete_form($form_state, $report, $condition_id) {
  if (empty($report->conditions[$condition_id])) {
    drupal_not_found();
    exit();
  }

  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Reports'), 'report'),
    l($report->title, 'report/' . $report->machine_name),
  );
  drupal_set_breadcrumb($breadcrumb);
  
  $form = array();
  $form['#report'] = $report;
  $form['#condition_id'] = $condition_id;

  $question = t('Are you sure you want to delete the condition <em>@name</em>?',
    array('@name' => $condition_id));
  $path = 'report/' . $report->machine_name . '/admin/condition/' . $condition_id . '/edit';

  return confirm_form($form, $question, $path);
}

function report_condition_delete_form_submit($form, &$form_state) {
  $report = $form['#report'];
  $condition_id = $form['#condition_id'];

  db_query("DELETE FROM {report_conditions} WHERE id = %d", $condition_id);
  $attr = array(
    '@id' => $condition_id,
  );
  drupal_set_message(t('The condition <em>@id</em> has been deleted successfully', $attr));
  watchdog('report', 'Field @id deleted', $attr);
  $form_state['redirect'] = 'report/' . $report->machine_name . '/admin';
}

function _resource_get_section_fields() {
  global $language;
  
  $list = array('' => t('- None -'));
  $sql = "SELECT sf.id, CONCAT(sd.title, ' - ', sfd.title) AS title FROM {section_fields} sf
    INNER JOIN {sections} s ON s.id = sf.section_id
    INNER JOIN {section_field_details} sfd ON sfd.field_id = sf.id AND sfd.language = '%s'
    INNER JOIN {section_details} sd ON sd.section_id = s.id AND sd.language = '%s'
    WHERE s.status = 1 AND sf.status = 1
    ORDER BY s.weight, sd.title, sf.weight, sfd.title";
  $result = db_query($sql, $language->language, $language->language);
  while ($data = db_fetch_object($result)) {
    $list[$data->id] = $data->title;
  }
  return $list;
}
