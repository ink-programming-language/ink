// Translated from solution.cpp.

var n: dynamic;

var s: dynamic;

var a = cpp_array(6, 6);

var ans = 8;

func parse(s: dynamic)
{
  var x = 0;
  var y = cpp_cast(((s[1] - cpp_char("1"))));
  if ((s[0] == cpp_char("G")))
  {
    x = 1;
  } else if ((s[0] == cpp_char("B")))
  {
    x = 2;
  } else if ((s[0] == cpp_char("Y")))
  {
    x = 3;
  } else if ((s[0] == cpp_char("W")))
  {
    x = 4;
  }
  a[x][y] = 1;
  return;
}

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(s);
      parse(s);
      i += 1;
    }
  }
  {
    var m1 = 0;
    while ((m1 < 32))
    {
      {
        var m2 = 0;
        while ((m2 < 32))
        {
          var k = 0;
          {
            var i = 0;
            while ((i < 5))
            {
              if ((m1 & ((1 << i))))
              {
                k += 1;
              }
              i += 1;
            }
          }
          {
            var i = 0;
            while ((i < 5))
            {
              if ((m2 & ((1 << i))))
              {
                k += 1;
              }
              i += 1;
            }
          }
          if ((k >= ans))
          {
            m2 += 1;
            continue;
          }
          var x = 0;
          {
            var i = 0;
            while ((i < 5))
            {
              {
                var j = 0;
                while ((j < 5))
                {
                  if (((a[i][j] && (((m1 & ((1 << i)))) == 0)) && (((m2 & ((1 << j)))) == 0)))
                  {
                    x += 1;
                  }
                  j += 1;
                }
              }
              i += 1;
            }
          }
          if ((x > 1))
          {
            m2 += 1;
            continue;
          }
          var good = 1;
          {
            var i = 0;
            while ((i < 5))
            {
              if ((((m1 & ((1 << i)))) == 0))
              {
                i += 1;
                continue;
              }
              var x = 0;
              {
                var j = 0;
                while ((j < 5))
                {
                  if ((a[i][j] && (((m2 & ((1 << j)))) == 0)))
                  {
                    x += 1;
                  }
                  j += 1;
                }
              }
              if ((x > 1))
              {
                good = 0;
              }
              i += 1;
            }
          }
          {
            var i = 0;
            while ((i < 5))
            {
              if ((((m2 & ((1 << i)))) == 0))
              {
                i += 1;
                continue;
              }
              var x = 0;
              {
                var j = 0;
                while ((j < 5))
                {
                  if ((a[j][i] && (((m1 & ((1 << j)))) == 0)))
                  {
                    x += 1;
                  }
                  j += 1;
                }
              }
              if ((x > 1))
              {
                good = 0;
              }
              i += 1;
            }
          }
          if (good)
          {
            ans = min(ans, k);
          }
          m2 += 1;
        }
      }
      m1 += 1;
    }
  }
  printf("%d\n", ans);
  read(n);
  return 0;
}
