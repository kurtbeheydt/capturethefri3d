<?php
$filename = 'data.json';
$handle = fopen($filename, 'r') or die('Cannot open file:  ' . $filename);
$data = json_decode(fread($handle,filesize($filename)));
fclose($handle);

function teamToColor($team) {
  $color;
  switch ($team) {
    case 'rood':
      $color = '#ffc9c9';
      break;
    case 'groen':
      $color ='#cdfecd';
      break;
    case 'blauw':
      $color = '#c8c8fc';
      break;
  }
  return $color;
}

// calculate total score
$totalscore = 0;
foreach ($data->scores as $team=>$score) {
  $totalscore = $totalscore + $score;
}
?>
  <!doctype html>
  <html lang="nl">

  <head>
    <!-- Required meta tags -->
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">

    <!-- Bootstrap CSS -->
    <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO"
      crossorigin="anonymous">

    <title>Capture The Fri3d</title>
    <style>
      body {
        background: url('bg_forest.jpg');
      }

      h1 {
        text-align: center;
      }

      h2 {
        margin-top: 20px;
        text-align: center;
        margin: 20px auto 10px auto;
        -webkit-text-stroke: 1px white;
        color: black;
        text-shadow: -1px -1px 0 #ffffff,
        1px -1px 0 #ffffff,
        -1px 1px 0 #ffffff,
        1px 1px 0 #ffffff;
      }

      .tower {
        border: 2px solid #222222;
      }

      .tower h3 {
        margin: 10px auto;
      }

      .teamName {
        text-align: center;
        margin: 5px auto 0px;
        text-shadow: -1px -1px 0 #111111,
        1px -1px 0 #111111,
        -1px 1px 0 #111111,
        1px 1px 0 #111111;
      }

      .leadingTitle {
        font-size: smaller;
        text-align: center;
        margin-top: 4px;
      }

      .leadingTeam {
        font-weight: bold;
        position: relative;
        top: -5px;
        font-size: larger;
      }
    </style>
  </head>

  <body>
    <div class="container">
      <h1>Capture the Fri3d</h1>
      <div class="row">
        <div class="col">
          <h2>Scores</h2>
          <div id="scores">
            <?php foreach ($data->scores as $team=>$score): ?>
            <div class="row">
              <div class="col">
                <div class="teamName" style="color: <?php echo teamToColor($team); ?>">team
                  <?php echo $team; ?>
                </div>
                <div>
                  <div class="progress" style="height: 30px; background-color: #ffffff; font-size: 1em; font-weight: strong;">
                    <div style="color: #111111; background-color: <?php echo teamToColor($team); ?>; width: <?php echo (100*$score/$totalscore); ?>%;"
                      class="progress-bar" role="progressbar" aria-valuenow="<?php echo $score; ?>" aria-valuemin="0" aria-valuemax="<?php echo $totalscore; ?>">
                      <?php echo $score; ?>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <?php endforeach; ?>
          </div>
        </div>
      </div>
      <div class="row">
        <div class="col">
          <h2>Torens</h2>
          <div class="row">
            <?php foreach ($data->towers as $tower=>$towerData): ?>
            <div class="col tower" style="margin: 5px; text-align: center; background-color: #ffffff;">
              <div class="row" style="background-color: <?php echo teamToColor($towerData->currentLeadingTeam); ?>">
                <div class="col">
                  <h3>
                    <?php echo $towerData->name; ?>
                  </h3>
                  <div class="leadingTitle">leider</div>
                  <div class="leadingTeam">
                    <?php echo $towerData->currentLeadingTeam; ?>
                  </div>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col">
                  <div class="leadingTitle">bezetting</div>
                </div>
              </div>
              <div class="row">
                <?php foreach ($towerData->teamData as $team=>$teamData): ?>
                <div class="col" style="text-align: center; background-color: <?php echo teamToColor($teamData->teamId); ?>">
                  <div style="text-align: center;">
                    <?php echo substr($teamData->teamId, 0, 1); ?>
                    <br />
                    <strong>
                      <?php echo $teamData->teamCount; ?>
                    </strong>
                  </div>
                </div>
                <?php endforeach; ?>
              </div>
            </div>
            <?php endforeach; ?>

          </div>
        </div>
      </div>
    </div>
    <!-- Optional JavaScript -->
    <!-- jQuery first, then Popper.js, then Bootstrap JS -->
    <script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo"
      crossorigin="anonymous"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.3/umd/popper.min.js" integrity="sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49"
      crossorigin="anonymous"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js" integrity="sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy"
      crossorigin="anonymous"></script>
  </body>

  </html>