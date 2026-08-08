// Translated from solution.cpp.

func find(j: dynamic)
{
  var s = "";
  while ((j > 0))
  {
    var ch = ((j % 10) + cpp_char("0"));
    s += ch;
    j = (j / 10);
  }
  reverse(s.begin(), s.end());
  return s;
}

func main()
{
  var s: dynamic;
  read(s);
  var n = s.size();
  var t = "";
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      t += s[i];
      i += 1;
    }
  }
  s.append(t);
  var arr = cpp_array(26, n, 26);
  {
    var i = 0;
    while ((i < 26))
    {
      {
        var j = 0;
        while ((j < n))
        {
          {
            var k = 0;
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
    var i = 0;
    while ((i < n))
    {
      {
        var j = 1;
        while ((j < n))
        {
          var c1 = (s[i] - cpp_char("a"));
          var c2 = (s[(i + j)] - cpp_char("a"));
          arr[c1][j][c2] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < 26))
    {
      var max = 0;
      {
        var j = 1;
        while ((j < n))
        {
          var x = 0;
          {
            var k = 0;
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
  var d = (cpp_cast(ans) / cpp_cast(n));
  write(fixed);
  write(setprecision(6));
  write(d, "\n");
}
