// Translated from solution.cpp.

var MAX: dynamic = (1e3 + 5);

var grid: dynamic = cpp_array(MAX, MAX);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var pos: dynamic = cpp_array(30);

var row: dynamic = cpp_array(MAX, MAX);

var col: dynamic = cpp_array(MAX, MAX);

func inside(x: dynamic, y: dynamic) -> dynamic
{
  return ((((x >= 1) && (x <= n)) && (y >= 1)) && (y <= m));
}

func row_wet(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.second > b.second))
  {
    swap(a, b);
  }
  return (row[a.first][b.second] - row[a.first][(a.second - 1)]);
}

func col_wet(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.first > b.first))
  {
    swap(a, b);
  }
  return (col[a.second][b.first] - col[a.second][(a.first - 1)]);
}

func main() -> dynamic
{
  memset(pos, -1, cpp_sizeof(pos));
  scanf("%d %d%*c", (&n), (&m));
  {
    var i: dynamic = int_cpp(1);
    while ((i < int_cpp((n + 1))))
    {
      {
        var j: dynamic = int_cpp(1);
        while ((j < int_cpp((m + 1))))
        {
          scanf("%c", (&grid[i][j]));
          if (((grid[i][j] >= cpp_char("A")) && (grid[i][j] <= cpp_char("Z"))))
          {
            pos[(grid[i][j] - cpp_char("A"))] = pair(i, j);
            grid[i][j] = cpp_char(".");
          }
          j += 1;
        }
      }
      scanf("%*c");
      i += 1;
    }
  }
  {
    var i: dynamic = int_cpp(1);
    while ((i < int_cpp((n + 1))))
    {
      {
        var j: dynamic = int_cpp(1);
        while ((j < int_cpp((m + 1))))
        {
          row[i][j] = (row[i][(j - 1)] + ((grid[i][j] == cpp_char("#"))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var j: dynamic = int_cpp(1);
    while ((j < int_cpp((m + 1))))
    {
      {
        var i: dynamic = int_cpp(1);
        while ((i < int_cpp((n + 1))))
        {
          col[j][i] = (col[j][(i - 1)] + ((grid[i][j] == cpp_char("#"))));
          i += 1;
        }
      }
      j += 1;
    }
  }
  var k: dynamic = cpp_uninitialized();
  scanf("%d%*c", (&k));
  var ins: dynamic = cpp_uninitialized();
  {
    var i: dynamic = int_cpp(0);
    while ((i < int_cpp(k)))
    {
      var c: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf("%c %d%*c", (&c), (&t));
      if ((c == cpp_char("N")))
      {
        ins.emplace_back((-t), 0);
      } else if ((c == cpp_char("S")))
      {
        ins.emplace_back(t, 0);
      } else if ((c == cpp_char("W")))
      {
        ins.emplace_back(0, (-t));
      } else if ((c == cpp_char("E")))
      {
        ins.emplace_back(0, t);
      }
      i += 1;
    }
  }
  var total: dynamic = 0;
  {
    var i: dynamic = int_cpp(0);
    while ((i < int_cpp(26)))
    {
      if ((pos[i].first == -1))
      {
        i += 1;
        continue;
      }
      var good: dynamic = true;
      var cur: dynamic = pos[i];
      for (var each: dynamic in ins)
      {
        var nxt: dynamic = pair((cur.first + each.first), (cur.second + each.second));
        if ((!inside(nxt.first, nxt.second)))
        {
          good = false;
          break;
        }
        if ((each.first == 0))
        {
          if (row_wet(cur, nxt))
          {
            good = false;
            break;
          }
        } else
        {
          if (col_wet(cur, nxt))
          {
            good = false;
          }
        }
        cur = nxt;
      }
      if (good)
      {
        printf("%c", (cpp_char("A") + i));
        total += 1;
      }
      i += 1;
    }
  }
  if ((total == 0))
  {
    puts("no solution");
  } else
  {
    puts("");
  }
  return 0;
}
