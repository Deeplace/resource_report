<?php

function report_list_page() {
  $output = '';
  
  $header = array();
  $rows = array();
  $is_admin = user_access('report admin');
  
  foreach (report_get_list() as $machine_name => $report) {
    if ($is_admin) {
      $title = empty($report['title']) ? $report['machine_name'] : $report['title'];
      $rows[] = array(
        l($title, 'report/' . $report['machine_name']),
        $report['status'] ? t('Enabled') : t('Disabled'),
        l(t('Manage'), 'report/' . $report['machine_name'] . '/admin'),
      );
    }
    elseif ($report['status'] && user_access('report access ' . $machine_name)) {
      $title = empty($report['title']) ? $report['machine_name'] : $report['title'];
      $rows[] = array(
        l($title, 'report/' . $report['machine_name']),
      );
    }
  }
  
  if ($is_admin) {
    $output .= '<div>' . l(t('Add report'), 'report/add') . '</div>';
  }
  
  if (empty($rows)) {
    $rows[] = array(
      t('No items found.'),
    );
  }

  $output .= theme('table', $header, $rows);
  
  return $output;
}

function report_view_page($report) {
  $output = '';
  $params = _report_get_params($report);
  $entity = 'report_' . $report->machine_name;
  
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Reports'), 'report'),
  );
  drupal_set_breadcrumb($breadcrumb);
  
  module_load_include('pages.inc', 'resource');
  if (!empty($params)) {
    // Form custom columns
    $columns = array();
    $default_columns = array();
    foreach ($report->columns as $column) {
      $default_columns[] = $column['field_id'] ? $column['field_id'] : $column['machine_name'];
      if (!$column['field_id']) {
        $columns[$column['machine_name']] = (object) $column;
      }
    }
    
    // FIXME: Remove this
    /*$columns['sexx'] = (object) array(
      'table_name' => 'resource',
      'machine_name' => 'sex',
      'title' => 'SEXxxXX',
      'type' => 'select',
      'source' => "1|Male\n0|Female",
    );
    $default_columns[] = 'sexx';*/
    
    // Set custom columns
    _resource_load_all_fields($columns);
    
    // Set default custom columns
    resource_get_selected_columns($entity, $default_columns);
    
    $output .= drupal_get_form('resource_filter_form', $entity);
    $output .= drupal_get_form('resource_set_columns_form', $entity);
  }
  $output .= drupal_get_form('report_params_form', $report, $params);
  
  if (!empty($params)) {
    $columns = resource_get_selected_columns($entity);
    $filter = empty($_SESSION['resource_filter'][$entity]) ? array() : $_SESSION['resource_filter'][$entity];
    
    $subquery = array();
    foreach ($report->conditions as $condition) {
      $condition['expression'] = strtr($condition['expression'], $params);
      $subquery[] = $condition;
    }
    
    $query = resource_generate_query($filter, $columns, '', $subquery);
    $table = resource_build_result_table($query);
    if (empty($table['rows'])) {
      $table['rows'][] = array(
        array('data' => t('No items found.'), 'colspan' => count($table['header'])),
      );
    }
    
    $q = $_GET;
    unset($q['q']);
    $output .= '<div class="export-icon">' .
      l(t('Export'), 'report/' . $report->machine_name . '/export', array('query' => $q)) .
      '</div>';
    $output .= theme('table', $table['header'], $table['rows']);
  }
  
  return $output;
  
}

function report_export_page($report) {
  module_load_include('pages.inc', 'resource');
  
  $params = _report_get_params($report);
  if (empty($params)) {
    drupal_not_found();
    exit();
  }
  
  $entity = 'report_' . $report->machine_name;
  $columns = array();
  foreach ($report->columns as $column) {
    if ($column['field_id']) {
      $columns[$column['field_id']] = $column['field_id'];
    }
    else {
      $columns[] = $column;
    }
  }
  foreach (resource_get_selected_columns($entity) as $key) {
    $columns[$key] = $key;
  }
  
  $filter = empty($_SESSION['resource_filter'][$entity]) ? array() : $_SESSION['resource_filter'][$entity];
  
  $subquery = array();
  foreach ($report->conditions as $condition) {
    $condition['expression'] = strtr($condition['expression'], $params);
    $subquery[] = $condition['expression'];
  }
  
  $query = resource_generate_query($filter, $columns, '', $subquery);
  $table = resource_build_result_table($query);
  resource_export_csv($table, 'export.csv');
}

function report_params_form($form_state, $report, $params) {
  $form = array();
  $form['#report'] = $report;
  
  $form['parameters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Report parameters'),
    '#collapsible' => TRUE,
    '#collapsed' => !empty($params),
  );
  
  foreach ($report->params as $param) {
    if (empty($form['parameters'][$param['category']])) {
      if (empty($param['category'])) {
        $form['parameters'][$param['category']] = array();
      }
      else {
        $form['parameters'][$param['category']] = array(
          '#type' => 'fieldset',
          '#title' => t($param['category']),
        );
      }
    }
    
    $obj = (object) $param;
    if ($param['type'] == 'date' || $param['type'] == 'timestamp') {
      $form['parameters'][$param['category']] += section_build_date_field($obj, $params['@' . $param['machine_name']]);
    }
    else {
      $form['parameters'][$param['category']] += section_build_generic_field($obj, array(), $params['@' . $param['machine_name']]);
    }
  }
  
  $form['parameters']['generate'] = array(
    '#type' => 'submit',
    '#default_value' => t('Generate'),
  );
  
  return $form;
}

function report_params_form_validate($form, &$form_state) {
  $report = $form['#report'];
  
  $placeholders = array();
  foreach ($report->params as $field) {
    if ($field['type'] == 'date') {
      if (module_exists('date_popup')) {
        $placeholders['@' . $field['machine_name']] = strtotime($form_state['values'][$field['machine_name']]);
      }
      else {
        $d = $form_state['values'][$field['machine_name']];
        $placeholders['@' . $field['machine_name']] = strtotime($d['year'] . '-' . $d['month'] . '-' . $d['day']);
      }
    }
    else {
      $placeholders['@' . $field['machine_name']] = "'" . addslashes($form_state['values'][$field['machine_name']]) . "'";
    }
  }
  
  if (!empty($report->validation)) {
    global $language;
    
    $lines = array_values(array_map('trim', explode("\n", strtr($report->validation, $placeholders))));
    foreach ($lines as $line) {
      $line = array_values(array_map('trim', explode('|', $line)));
      if (count($line) >= 2) {
        $code = 'return ' . array_shift($line) . ';';
        $message = array('' => '');
        foreach ($line as $item) {
          $item = array_map('trim', explode(':' , $item, 2));
          if (count($item) == 1) {
            $message[''] = t($item[0]);
          }
          else {
            $message[$item[0]] = $item[1];
          }
        }
        
        if (eval($code)) {
          form_set_error('', empty($message[$language->language]) ? $message[''] : $message[$language->language]);
        }
      }
    }
  }
}

function report_params_form_submit($form, &$form_state) {
  $report = $form['#report'];
  
  $params = array();
  foreach ($report->params as $param) {
    if (!empty($form_state['values'][$param['machine_name']])) {
      $params[$param['machine_name']] = $form_state['values'][$param['machine_name']];
      if ($param['type'] == 'date') {
        $params[$param['machine_name']]= date('Y-m-d', strtotime($params[$param['machine_name']]));
      }
      elseif ($param['type'] == 'timestamp') {
        $params[$param['machine_name']]= strtotime($params[$param['machine_name']]);
      }
    }
  }
  
  $form_state['redirect'] = array($_GET['q'], $params);
}

function _report_get_params($report) {
  $params = array();
  foreach ($report->params as $param) {
    $params['@' . $param['machine_name']] = empty($_GET[$param['machine_name']]) ? '' : $_GET[$param['machine_name']];
  }
  
  $temp = array_filter($params);
  return empty($temp) ? array() : $params;
}
