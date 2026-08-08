// Translated from solution.cpp.

func mini(a4: dynamic, b4: dynamic)
{
  a4 = min(a4, b4);
}

func maxi(a4: dynamic, b4: dynamic)
{
  a4 = max(a4, b4);
}

func dbg(sdbg: dynamic, h: dynamic)
{
  write(sdbg, "=", h, "\n");
}

func dbg(sdbg: dynamic, h: dynamic, a: dynamic...)
{
  while (((*sdbg) != cpp_char(",")))
  {
    write((*cpp_update(sdbg, "++")));
  }
  write("=", h, ",");
  dbg((sdbg + 1), cpp_expand(a));
}

func operator_shift_left(os: dynamic, V: dynamic)
{
  (os << "[");
  for (var vv in V)
  {
    ((os << vv) << ",");
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, P: dynamic)
{
  return (((((os << "(") << P.first) << ",") << P.second) << ")");
}

var MAX = 51;

var t = cpp_array(MAX);

var n: dynamic;

var m: dynamic;

func licz(x: dynamic)
{
  var bil = 0;
  {
    var i = (0);
    while ((i <= ((cpp_cast((n)) - 1))))
    {
      read(t[i]);
      {
        var j = (0);
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
  var res: dynamic;
  var co = 0;
  while ((bil != x))
  {
    if (co)
    {
      {
        var i = (0);
        while ((i <= ((cpp_cast(((n - 1))) - 1))))
        {
          {
            var j = (0);
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
        var i = (0);
        while ((i <= ((cpp_cast(((n - 1))) - 1))))
        {
          {
            var j = (0);
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

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  write(fixed, setprecision(11));
  if (0)
  {
    write(fixed, setprecision(6));
  }
  read(n, m);
  var pom = ((n * m) / 2);
  if ((n & 1))
  {
    pom *= -1;
  }
  (pom);
  var a = licz(pom);
  ("xxx");
  var b = licz(pom);
  write(((cpp_cast((a).size())) + (cpp_cast((b).size()))), "\n");
  for (var el in a)
  {
    write((el.first + 1), " ", (el.second + 1), "\n");
  }
  reverse((b).begin(), (b).end());
  for (var el in b)
  {
    write((el.first + 1), " ", (el.second + 1), "\n");
  }
}
