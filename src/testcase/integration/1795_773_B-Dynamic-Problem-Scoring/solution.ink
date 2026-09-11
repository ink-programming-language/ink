// Translated from solution.cpp.

var MAX_D: dynamic = 1e5;

var TASK_COUNT: dynamic = 5;

var STATE_COUNT: dynamic = 6;

var COEF: dynamic = [2, 4, 8, 16, 32];

var SCORE: dynamic = [500, 1000, 1500, 2000, 2500, 3000];

var n: dynamic = cpp_uninitialized();

var answer: dynamic = MAX_D;

var s: dynamic = cpp_uninitialized();

var success: dynamic = cpp_uninitialized();

var taskScore: dynamic = cpp_uninitialized();

var successBuffer: dynamic = cpp_uninitialized();

var precalcTotal: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

func UpdateAnswer(d: dynamic) -> dynamic
{
  var total: dynamic = (d + n);
  successBuffer.assign(TASK_COUNT, 0);
  {
    var i: dynamic = 0;
    while ((i < TASK_COUNT))
    {
      if ((a[0][i] == -1))
      {
        i += 1;
        continue;
      }
      if ((s[i] == 5))
      {
        successBuffer[i] = 0;
      } else
      {
        var x: dynamic = (total - (COEF[s[i]] * success[i]));
        if ((x < 0))
        {
          successBuffer[i] = 0;
        } else
        {
          successBuffer[i] = ((x / COEF[s[i]]) + 1);
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < TASK_COUNT))
    {
      if ((successBuffer[i] > d))
      {
        return;
      }
      i += 1;
    }
  }
  taskScore.assign(TASK_COUNT, 0);
  {
    var i: dynamic = 0;
    while ((i < TASK_COUNT))
    {
      var x: dynamic = (success[i] + successBuffer[i]);
      if ((total < (x * 2)))
      {
        taskScore[i] = 500;
      } else if ((total < (x * 4)))
      {
        taskScore[i] = 1000;
      } else if ((total < (x * 8)))
      {
        taskScore[i] = 1500;
      } else if ((total < (x * 16)))
      {
        taskScore[i] = 2000;
      } else if ((total < (x * 32)))
      {
        taskScore[i] = 2500;
      } else
      {
        taskScore[i] = 3000;
      }
      i += 1;
    }
  }
  var score: dynamic = cpp_array(2);
  score[0] = cpp_assign(score[1], "=", 0);
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      {
        var j: dynamic = 0;
        while ((j < TASK_COUNT))
        {
          if ((a[i][j] != -1))
          {
            score[i] += (taskScore[j] - ((a[i][j] * taskScore[j]) / 250));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((score[0] > score[1]))
  {
    answer = min(answer, d);
  }
}

func UpdateAnswer() -> dynamic
{
  {
    var d: dynamic = 0;
    while ((d < MAX_D))
    {
      UpdateAnswer(d);
      d += 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read(n);
  a.resize(n, vector(TASK_COUNT, -1));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < TASK_COUNT))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  success.resize(TASK_COUNT, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < TASK_COUNT))
        {
          if ((a[i][j] >= 0))
          {
            success[j] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var d: dynamic = 0;
    while ((d <= MAX_D))
    {
      var total: dynamic = (n + d);
      successBuffer.assign(TASK_COUNT, 0);
      {
        var i: dynamic = 0;
        while ((i < TASK_COUNT))
        {
          if ((((a[1][i] != -1) && (a[0][i] != -1)) && (a[1][i] < a[0][i])))
          {
            successBuffer[i] = d;
          }
          i += 1;
        }
      }
      taskScore.assign(TASK_COUNT, 0);
      {
        var i: dynamic = 0;
        while ((i < TASK_COUNT))
        {
          var x: dynamic = (success[i] + successBuffer[i]);
          if ((total < (x * 2)))
          {
            taskScore[i] = 500;
          } else if ((total < (x * 4)))
          {
            taskScore[i] = 1000;
          } else if ((total < (x * 8)))
          {
            taskScore[i] = 1500;
          } else if ((total < (x * 16)))
          {
            taskScore[i] = 2000;
          } else if ((total < (x * 32)))
          {
            taskScore[i] = 2500;
          } else
          {
            taskScore[i] = 3000;
          }
          i += 1;
        }
      }
      var score: dynamic = cpp_array(2);
      score[0] = cpp_assign(score[1], "=", 0);
      {
        var i: dynamic = 0;
        while ((i < 2))
        {
          {
            var j: dynamic = 0;
            while ((j < TASK_COUNT))
            {
              if ((a[i][j] != -1))
              {
                score[i] += (taskScore[j] - ((a[i][j] * taskScore[j]) / 250));
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      if ((score[0] > score[1]))
      {
        answer = min(answer, d);
      }
      d += 1;
    }
  }
  if ((answer == MAX_D))
  {
    write(-1, "\n");
  } else
  {
    write(answer, "\n");
  }
  return 0;
}
