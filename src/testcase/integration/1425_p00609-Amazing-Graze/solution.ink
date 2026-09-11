// Translated from solution.cpp.

func S(X: dynamic) -> dynamic
{
  return cpp_expression("//32 #inc");
}

func main() -> dynamic
{
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    while (cpp_comma((((cin >> a) >> b) >> r), ((a | b) | r)))
    {
      var x: dynamic = cpp_array(100000);
      var y: dynamic = cpp_array(100000);
      {
        var i: dynamic = 0;
        while ((i < a))
        {
          read(x[i], y[i]);
          i += 1;
        }
      }
      var v: dynamic = cpp_array(250, 250);
      {
        var i: dynamic = 0;
        while ((i < b))
        {
          var xb: dynamic = cpp_uninitialized();
          var yb: dynamic = cpp_uninitialized();
          read(xb, yb);
          v[(yb / 40)][(xb / 40)].push_back(pii(yb, xb));
          i += 1;
        }
      }
      var s: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < a))
        {
          var yy: dynamic = (y[i] / 40);
          var xx: dynamic = (x[i] / 40);
          {
            var j: dynamic = -1;
            while ((j <= 1))
            {
              {
                var k: dynamic = -1;
                while ((k <= 1))
                {
                  var yt: dynamic = (yy + j);
                  var xt: dynamic = (xx + k);
                  if (((((0 <= yt) && (yt < 250)) && (0 <= xt)) && (xt < 250)))
                  {
                    {
                      var l: dynamic = 0;
                      while ((l < v[yt][xt].size()))
                      {
                        s += ((S((y[i] - v[yt][xt][l].first)) + S((x[i] - v[yt][xt][l].second))) <= S((4 * r)));
                        l += 1;
                      }
                    }
                  }
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      write(s, "\n");
    }
  }
  return 0;
}
