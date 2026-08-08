// Translated from solution.cpp.

var EPS = cpp_expression("#inc");

var PI = cpp_expression("#include");

func EQ(a: dynamic, b: dynamic)
{
  return cpp_expression("#include<iostream>");
}

var house: dynamic;

var ume: dynamic;

var sak: dynamic;

var mo: dynamic;

func cross(v1: dynamic, v2: dynamic)
{
  return ((v1.real() * v2.imag()) - (v1.imag() * v2.real()));
}

func che(h: dynamic, t: dynamic, d: dynamic, w: dynamic, a: dynamic)
{
  var v = (h - t);
  if ((!((abs(v) < a))))
  {
    return false;
  }
  var v1 = P((a * cos(((((w + (d / 2))) / 180) * PI))), (a * sin(((((w + (d / 2))) / 180) * PI))));
  var v2 = P((a * cos(((((w - (d / 2))) / 180) * PI))), (a * sin(((((w - (d / 2))) / 180) * PI))));
  if (((cross(v1, v) < 0) && (cross(v2, v) > 0)))
  {
    return true;
  } else
  {
    return false;
  }
}

func main()
{
  var h: dynamic;
  var r: dynamic;
  while (((((cin >> h) >> r) && h) && r))
  {
    house.clear();
    ume.push_back(P(0, 0));
    {
      var i = 0;
      while ((i < h))
      {
        var x: dynamic;
        var y: dynamic;
        read(x, y);
        house.push_back(P(x, y));
        i += 1;
      }
    }
    var U: dynamic;
    var M: dynamic;
    var S: dynamic;
    var du: dynamic;
    var dm: dynamic;
    var ds: dynamic;
    read(U, M, S, du, dm, ds);
    {
      var i = 0;
      while ((i < U))
      {
        var x: dynamic;
        var y: dynamic;
        read(x, y);
        ume.push_back(P(x, y));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < M))
      {
        var x: dynamic;
        var y: dynamic;
        read(x, y);
        mo.push_back(P(x, y));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < S))
      {
        var x: dynamic;
        var y: dynamic;
        read(x, y);
        sak.push_back(P(x, y));
        i += 1;
      }
    }
    var data = cpp_array(120);
    {
      var i = 0;
      while ((i < 120))
      {
        data[i] = 0;
        i += 1;
      }
    }
    {
      var z = 0;
      while ((z < r))
      {
        var w: dynamic;
        var a: dynamic;
        read(w, a);
        {
          var i = 0;
          while ((i < house.size()))
          {
            var ok = false;
            if (che(house[i], ume[0], du, w, a))
            {
              ok = true;
            } else
            {
              i += 1;
              continue;
            }
            {
              var j = 1;
              while ((j <= U))
              {
                if (che(house[i], ume[j], du, w, a))
                {
                  ok = false;
                }
                j += 1;
              }
            }
            {
              var j = 0;
              while ((j < M))
              {
                if (che(house[i], mo[j], dm, w, a))
                {
                  ok = false;
                }
                j += 1;
              }
            }
            {
              var j = 0;
              while ((j < S))
              {
                if (che(house[i], sak[j], ds, w, a))
                {
                  ok = false;
                }
                j += 1;
              }
            }
            if (ok)
            {
              data[i] += 1;
            }
            i += 1;
          }
        }
        z += 1;
      }
    }
    var ans = 0;
    var tmp = -1;
    {
      var i = 0;
      while ((i < h))
      {
        if ((ans <= data[i]))
        {
          ans = data[i];
          tmp = i;
        }
        i += 1;
      }
    }
    if ((ans == 0))
    {
      write("NA", "\n");
    } else
    {
      {
        var i = 0;
        while ((i < h))
        {
          if ((i == tmp))
          {
            write((i + 1));
          } else if ((data[i] == ans))
          {
            write((i + 1), " ");
          }
          i += 1;
        }
      }
      write("\n");
    }
  }
}
