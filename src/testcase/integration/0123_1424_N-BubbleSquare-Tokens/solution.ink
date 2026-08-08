// Translated from solution.cpp.

func gc(argument_0: dynamic)
{
  var buf = cpp_array(100000);
  var p1 = buf;
  var p2 = buf;
  return if (((p1 == p2) && (cpp_assign(p2, "=", (p1 == p2))))) EOF else (*cpp_update(p1, "++"));
}

func read(x: dynamic)
{
  var flag = cpp_cast(1);
  x = 0;
  var ch = gc();
  {
    while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
    {
      flag = if ((ch == cpp_char("-"))) -1 else 1;
      ch = gc();
    }
  }
  {
    while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
    {
      x = ((((x << 1)) + ((x << 3))) + ((ch & 15)));
      ch = gc();
    }
  }
  x *= flag;
  return;
}

func readstr(x: dynamic)
{
  x = "";
  var ch: dynamic;
  while (isspace(cpp_assign(ch, "=", gc())))
  {
  }
  while (cpp_comma(cpp_assign(x, "+=", ch), (!isspace(cpp_assign(ch, "=", gc())))))
  {
  }
}

func readstr(s: dynamic)
{
  while (true)
  {
    (*s) = gc();
    if (!(((((((*s) == cpp_char(" "))) || (((*s) == cpp_char("\n")))) || (((*s) == cpp_char("\r")))))))
    {
      break;
    }
  }
  while (true)
  {
    (*(cpp_update(s, "++"))) = gc();
    if (!(((((((~(*s))) && (((*s) != cpp_char(" ")))) && (((*s) != cpp_char("\n")))) && (((*s) != cpp_char("\r")))))))
    {
      break;
    }
  }
  (*s) = 0;
  return;
}

func printstr(x: dynamic, num: dynamic = 0, ch: dynamic = cpp_char("\n"))
{
  {
    var i = num;
    while ((i < x.size()))
    {
      putchar(x[i]);
      i += 1;
    }
  }
  putchar(ch);
}

func readch(x: dynamic)
{
  while (isspace(cpp_assign(x, "=", gc())))
  {
  }
}

var pf = cpp_array(100000);

var o1 = pf;

var o2 = (pf + 100000);

func println(x: dynamic, c: dynamic = cpp_char("\n"))
{
  if ((x < 0))
  {
    (cpp_assign(if ((o1 == o2)) cpp_comma(fwrite(pf, 1, 100000, stdout), cpp_assign((*cpp_update((cpp_assign(o1, "=", pf)), "++")), "=", 45)) else (*cpp_update(o1, "++")), "=", 45));
    x = (-x);
  }
  var s = cpp_array(15);
  var b: dynamic;
  b = s;
  if ((!x))
  {
    (*cpp_update(b, "++")) = 48;
  }
  {
    while (x)
    {
      (*cpp_update(b, "++")) = ((x % 10) + 48);
      x /= 10;
    }
  }
  {
    while ((cpp_update(b, "--") != s))
    {
      (cpp_assign(if ((o1 == o2)) cpp_comma(fwrite(pf, 1, 100000, stdout), cpp_assign((*cpp_update((cpp_assign(o1, "=", pf)), "++")), "=", (*b))) else (*cpp_update(o1, "++")), "=", (*b)));
    }
  }
  (cpp_assign(if ((o1 == o2)) cpp_comma(fwrite(pf, 1, 100000, stdout), cpp_assign((*cpp_update((cpp_assign(o1, "=", pf)), "++")), "=", c)) else (*cpp_update(o1, "++")), "=", c));
}

var wbuf = cpp_array(25);

var wl = 0;

func write(x: dynamic)
{
  if ((x == 0))
  {
    putchar(48);
    return;
  }
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  wl = 0;
  while (x)
  {
    wbuf[cpp_update(wl, "++")] = (x % 10);
    x /= 10;
  }
  {
    var i = wl;
    while ((i >= 1))
    {
      putchar((wbuf[i] + 48));
      i -= 1;
    }
  }
}

func writeln(x: dynamic)
{
  write(x);
  puts("");
}

func writeln(x: dynamic, c: dynamic)
{
  write(x);
  putchar(c);
}

func writeln(c: dynamic, x: dynamic)
{
  putchar(c);
  write(x);
}

func chkmax(x: dynamic, y: dynamic)
{
  if ((x > y)) cpp_assign(x, "=", x) else cpp_assign(x, "=", y);
}

func chkmin(x: dynamic, y: dynamic)
{
  if ((x < y)) cpp_assign(x, "=", x) else cpp_assign(x, "=", y);
}

func max(x: dynamic, y: dynamic, z: dynamic)
{
  return if ((x > y)) (if ((x > z)) x else z) else (if ((y > z)) y else z);
}

func file(str: dynamic)
{
  freopen(((str + ".in")).c_str(), "r", stdin);
  freopen(((str + ".out")).c_str(), "w", stdout);
}

class Vector
{
  var x: dynamic;
  var y: dynamic;
  func Vector(x: dynamic = 0, y: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func Vary(argument_0: dynamic)
  {
      return Vector(x, (-y));
    }
  func operator_less(rhs: dynamic)
  {
      return if ((x == rhs.x)) (y < rhs.y) else (x < rhs.x);
    }
  func operator_subtract(rhs: dynamic)
  {
      return Vector((x - rhs.x), (y - rhs.y));
    }
  func operator_add(rhs: dynamic)
  {
      return Vector((x + rhs.x), (y + rhs.y));
    }
  func operator_multiply(rhs: dynamic)
  {
      return Vector((x * rhs), (y * rhs));
    }
  func operator_divide(rhs: dynamic)
  {
      return Vector((x / rhs), (y / rhs));
    }
  func operator_multiply(rhs: dynamic)
  {
      return Vector(((x * rhs.x) - (y * rhs.y)), ((x * rhs.y) + (y * rhs.x)));
    }
}

var N = (1e6 + 100);

var x = cpp_array(N);

var y = cpp_array(N);

var w = cpp_array(N);

var s = cpp_array(N);

var v = cpp_array(N);

var in_cpp = cpp_array((N * 2));

var e = cpp_array(N);

var res: dynamic;

var n: dynamic;

var m: dynamic;

func main()
{
  read(n);
  read(m);
  {
    var i = (1);
    while ((i <= (m)))
    {
      read(x[i]);
      read(y[i]);
      w[i] = 1;
      s[x[i]] += 1;
      s[y[i]] += 1;
      e[max(x[i], y[i])].push_back(make_pair(min(x[i], y[i]), i));
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= (n)))
    {
      {
        var it = (0);
        while ((it < (int_cpp(e[i].size()))))
        {
          if ((!v[e[i][it].first]))
          {
            v[e[i][it].first] = 1;
            w[e[i][it].second] = 0;
            s[i] -= 1;
          }
          in_cpp[s[e[i][it].first]] = 1;
          it += 1;
        }
      }
      {
        var it = (0);
        while ((it < (int_cpp(e[i].size()))))
        {
          if ((!in_cpp[s[i]]))
          {
            break;
          }
          s[i] += 1;
          v[e[i][it].first] = 0;
          w[e[i][it].second] += 1;
          it += 1;
        }
      }
      {
        var it = (0);
        while ((it < (int_cpp(e[i].size()))))
        {
          in_cpp[s[e[i][it].first]] = 0;
          it += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= (n)))
    {
      if (v[i])
      {
        res.push_back(i);
      }
      i += 1;
    }
  }
  writeln(int_cpp(res.size()));
  {
    var it = (0);
    while ((it < (int_cpp(res.size()))))
    {
      writeln(res[it], cpp_char(" "));
      it += 1;
    }
  }
  puts("");
  {
    var i = (1);
    while ((i <= (m)))
    {
      writeln(x[i], cpp_char(" "));
      writeln(y[i], cpp_char(" "));
      writeln(w[i]);
      i += 1;
    }
  }
  return 0;
}
