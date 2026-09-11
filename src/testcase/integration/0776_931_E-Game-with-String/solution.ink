// Translated from solution.cpp.

func find(j: dynamic) -> dynamic
{
  var s: dynamic = "";
  while ((j > 0))
  {
    var ch: dynamic = ((j % 10) + cpp_char("0"));
    s += ch;
    j = (j / 10);
  }
  reverse(s.begin(), s.end());
  return s;
}

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var n: dynamic = s.size();
  var t: dynamic = "";
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      t += s[i];
      i += 1;
    }
  }
  s.append(t);
  var arr: dynamic = cpp_array(26, n, 26);
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          {
            var k: dynamic = 0;
            while ((k < 26))
            {
              arr[i][j][k] = 0;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 1;
        while ((j < n))
        {
          var c1: dynamic = (s[i] - cpp_char("a"));
          var c2: dynamic = (s[(i + j)] - cpp_char("a"));
          arr[c1][j][c2] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      var max: dynamic = 0;
      {
        var j: dynamic = 1;
        while ((j < n))
        {
          var x: dynamic = 0;
          {
            var k: dynamic = 0;
            while ((k < 26))
            {
              if ((arr[i][j][k] == 1))
              {
                x += 1;
              }
              k += 1;
            }
          }
          if ((x > max))
          {
            max = x;
          }
          j += 1;
        }
      }
      ans += max;
      i += 1;
    }
  }
  var d: dynamic = (cpp_cast(ans) / cpp_cast(n));
  write(fixed);
  write(setprecision(6));
  write(d, "\n");
}
