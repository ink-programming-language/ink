// Translated from solution.cpp.

func mini(a4: dynamic, b4: dynamic) -> dynamic
{
  a4 = min(a4, b4);
}

func maxi(a4: dynamic, b4: dynamic) -> dynamic
{
  a4 = max(a4, b4);
}

func dbg(sdbg: dynamic, h: dynamic) -> dynamic
{
  write(sdbg, "=", h, "\n");
}

func dbg(sdbg: dynamic, h: dynamic, a: dynamic...) -> dynamic
{
  while (((*sdbg) != cpp_char(",")))
  {
    write((*cpp_update(sdbg, "++")));
  }
  write("=", h, ",");
  dbg((sdbg + 1), cpp_expand(a));
}

func operator_shift_left(os: dynamic, V: dynamic) -> dynamic
{
  (os << "[");
  for (var vv: dynamic in V)
  {
    ((os << vv) << ",");
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, P: dynamic) -> dynamic
{
  return (((((os << "(") << P.first) << ",") << P.second) << ")");
}

var MAX: dynamic = 51;

var t: dynamic = cpp_array(MAX);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func licz(x: dynamic) -> dynamic
{
  var bil: dynamic = 0;
  {
    var i: dynamic = (0);
    while ((i <= ((cpp_cast((n)) - 1))))
    {
      read(t[i]);
      {
        var j: dynamic = (0);
        while ((j <= ((cpp_cast((m)) - 1))))
        {
          if ((t[i][j] == cpp_char("U")))
          {
            bil += 1;
          }
          if ((t[i][j] == cpp_char("L")))
          {
            bil -= 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var res: dynamic = cpp_uninitialized();
  var co: dynamic = 0;
  while ((bil != x))
  {
    if (co)
    {
      {
        var i: dynamic = (0);
        while ((i <= ((cpp_cast(((n - 1))) - 1))))
        {
          {
            var j: dynamic = (0);
            while ((j <= ((cpp_cast(((m - 1))) - 1))))
            {
              if (((((t[i][j] == cpp_char("L")) && (t[i][(j + 1)] == cpp_char("R"))) && (t[(i + 1)][j] == cpp_char("L"))) && (t[(i + 1)][(j + 1)] == cpp_char("R"))))
              {
                t[i][j] = cpp_assign(t[i][(j + 1)], "=", cpp_char("U"));
                t[(i + 1)][j] = cpp_assign(t[(i + 1)][(j + 1)], "=", cpp_char("D"));
                bil += 4;
                res.push_back([i, j]);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    } else
    {
      {
        var i: dynamic = (0);
        while ((i <= ((cpp_cast(((n - 1))) - 1))))
        {
          {
            var j: dynamic = (0);
            while ((j <= ((cpp_cast(((m - 1))) - 1))))
            {
              if (((((t[i][j] == cpp_char("U")) && (t[i][(j + 1)] == cpp_char("U"))) && (t[(i + 1)][j] == cpp_char("D"))) && (t[(i + 1)][(j + 1)] == cpp_char("D"))))
              {
                t[i][j] = cpp_char("L");
                t[i][(j + 1)] = cpp_char("R");
                t[(i + 1)][j] = cpp_char("L");
                t[(i + 1)][(j + 1)] = cpp_char("R");
                bil -= 4;
                res.push_back([i, j]);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
    co = (!co);
  }
  return res;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  write(fixed, setprecision(11));
  if (0)
  {
    write(fixed, setprecision(6));
  }
  read(n, m);
  var pom: dynamic = ((n * m) / 2);
  if ((n & 1))
  {
    pom *= -1;
  }
  (pom);
  var a: dynamic = licz(pom);
  ("xxx");
  var b: dynamic = licz(pom);
  write(((cpp_cast((a).size())) + (cpp_cast((b).size()))), "\n");
  for (var el: dynamic in a)
  {
    write((el.first + 1), " ", (el.second + 1), "\n");
  }
  reverse((b).begin(), (b).end());
  for (var el: dynamic in b)
  {
    write((el.first + 1), " ", (el.second + 1), "\n");
  }
}
