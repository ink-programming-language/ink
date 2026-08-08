// Translated from solution.cpp.

func S(X: dynamic)
{
  return cpp_expression("//32 #inc");
}

func main()
{
  {
    var a: dynamic;
    var b: dynamic;
    var r: dynamic;
    while (cpp_comma((((cin >> a) >> b) >> r), ((a | b) | r)))
    {
      var x = cpp_array(100000);
      var y = cpp_array(100000);
      {
        var i = 0;
        while ((i < a))
        {
          read(x[i], y[i]);
          i += 1;
        }
      }
      var v = cpp_array(250, 250);
      {
        var i = 0;
        while ((i < b))
        {
          var xb: dynamic;
          var yb: dynamic;
          read(xb, yb);
          v[(yb / 40)][(xb / 40)].push_back(pii(yb, xb));
          i += 1;
        }
      }
      var s = 0;
      {
        var i = 0;
        while ((i < a))
        {
          var yy = (y[i] / 40);
          var xx = (x[i] / 40);
          {
            var j = -1;
            while ((j <= 1))
            {
              {
                var k = -1;
                while ((k <= 1))
                {
                  var yt = (yy + j);
                  var xt = (xx + k);
                  if (((((0 <= yt) && (yt < 250)) && (0 <= xt)) && (xt < 250)))
                  {
                    {
                      var l = 0;
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
