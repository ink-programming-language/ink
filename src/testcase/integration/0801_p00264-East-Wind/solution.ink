// Translated from solution.cpp.

var EPS: dynamic = cpp_expression("#inc");

var PI: dynamic = cpp_expression("#include");

func EQ(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream>");
}

var house: dynamic = cpp_uninitialized();

var ume: dynamic = cpp_uninitialized();

var sak: dynamic = cpp_uninitialized();

var mo: dynamic = cpp_uninitialized();

func cross(v1: dynamic, v2: dynamic) -> dynamic
{
  return ((v1.real() * v2.imag()) - (v1.imag() * v2.real()));
}

func che(h: dynamic, t: dynamic, d: dynamic, w: dynamic, a: dynamic) -> dynamic
{
  var v: dynamic = (h - t);
  if ((!((abs(v) < a))))
  {
    return false;
  }
  var v1: dynamic = P((a * cos(((((w + (d / 2))) / 180) * PI))), (a * sin(((((w + (d / 2))) / 180) * PI))));
  var v2: dynamic = P((a * cos(((((w - (d / 2))) / 180) * PI))), (a * sin(((((w - (d / 2))) / 180) * PI))));
  if (((cross(v1, v) < 0) && (cross(v2, v) > 0)))
  {
    return true;
  } else
  {
    return false;
  }
}

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  while (((((cin >> h) >> r) && h) && r))
  {
    house.clear();
    ume.push_back(P(0, 0));
    {
      var i: dynamic = 0;
      while ((i < h))
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
        read(x, y);
        house.push_back(P(x, y));
        i += 1;
      }
    }
    var U: dynamic = cpp_uninitialized();
    var M: dynamic = cpp_uninitialized();
    var S: dynamic = cpp_uninitialized();
    var du: dynamic = cpp_uninitialized();
    var dm: dynamic = cpp_uninitialized();
    var ds: dynamic = cpp_uninitialized();
    read(U, M, S, du, dm, ds);
    {
      var i: dynamic = 0;
      while ((i < U))
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
        read(x, y);
        ume.push_back(P(x, y));
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < M))
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
        read(x, y);
        mo.push_back(P(x, y));
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < S))
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
        read(x, y);
        sak.push_back(P(x, y));
        i += 1;
      }
    }
    var data: dynamic = cpp_array(120);
    {
      var i: dynamic = 0;
      while ((i < 120))
      {
        data[i] = 0;
        i += 1;
      }
    }
    {
      var z: dynamic = 0;
      while ((z < r))
      {
        var w: dynamic = cpp_uninitialized();
        var a: dynamic = cpp_uninitialized();
        read(w, a);
        {
          var i: dynamic = 0;
          while ((i < house.size()))
          {
            var ok: dynamic = false;
            if (che(house[i], ume[0], du, w, a))
            {
              ok = true;
            } else
            {
              i += 1;
              continue;
            }
            {
              var j: dynamic = 1;
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
              var j: dynamic = 0;
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
              var j: dynamic = 0;
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
    var ans: dynamic = 0;
    var tmp: dynamic = -1;
    {
      var i: dynamic = 0;
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
        var i: dynamic = 0;
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
