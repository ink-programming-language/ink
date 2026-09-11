// Translated from solution.cpp.

func gc(argument_0: dynamic) -> dynamic
{
  var buf: dynamic = cpp_array(100000);
  var p1: dynamic = buf;
  var p2: dynamic = buf;
  return  (((p1 == p2) && (cpp_assign(p2, "=", (p1 == p2))))) ? EOF : (*cpp_update(p1, "++"));
}

func read(x: dynamic) -> dynamic
{
  var flag: dynamic = cpp_cast(1);
  x = 0;
  var ch: dynamic = gc();
  {
    while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
    {
      flag =  ((ch == cpp_char("-"))) ? -1 : 1;
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

func readstr(x: dynamic) -> dynamic
{
  x = "";
  var ch: dynamic = cpp_uninitialized();
  while (isspace(cpp_assign(ch, "=", gc())))
  {
  }
  while (cpp_comma(cpp_assign(x, "+=", ch), (!isspace(cpp_assign(ch, "=", gc())))))
  {
  }
}

func readstr(s: dynamic) -> dynamic
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

func printstr(x: dynamic, num: dynamic = 0, ch: dynamic = cpp_char("\n")) -> dynamic
{
  {
    var i: dynamic = num;
    while ((i < x.size()))
    {
      putchar(x[i]);
      i += 1;
    }
  }
  putchar(ch);
}

func readch(x: dynamic) -> dynamic
{
  while (isspace(cpp_assign(x, "=", gc())))
  {
  }
}

var pf: dynamic = cpp_array(100000);

var o1: dynamic = pf;

var o2: dynamic = (pf + 100000);

func println(x: dynamic, c: dynamic = cpp_char("\n")) -> dynamic
{
  if ((x < 0))
  {
    (cpp_assign( ((o1 == o2)) ? cpp_comma(fwrite(pf, 1, 100000, stdout), cpp_assign((*cpp_update((cpp_assign(o1, "=", pf)), "++")), "=", 45)) : (*cpp_update(o1, "++")), "=", 45));
    x = (-x);
  }
  var s: dynamic = cpp_array(15);
  var b: dynamic = cpp_uninitialized();
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
      (cpp_assign( ((o1 == o2)) ? cpp_comma(fwrite(pf, 1, 100000, stdout), cpp_assign((*cpp_update((cpp_assign(o1, "=", pf)), "++")), "=", (*b))) : (*cpp_update(o1, "++")), "=", (*b)));
    }
  }
  (cpp_assign( ((o1 == o2)) ? cpp_comma(fwrite(pf, 1, 100000, stdout), cpp_assign((*cpp_update((cpp_assign(o1, "=", pf)), "++")), "=", c)) : (*cpp_update(o1, "++")), "=", c));
}

var wbuf: dynamic = cpp_array(25);

var wl: dynamic = 0;

func write(x: dynamic) -> dynamic
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
    var i: dynamic = wl;
    while ((i >= 1))
    {
      putchar((wbuf[i] + 48));
      i -= 1;
    }
  }
}

func writeln(x: dynamic) -> dynamic
{
  write(x);
  puts("");
}

func writeln(x: dynamic, c: dynamic) -> dynamic
{
  write(x);
  putchar(c);
}

func writeln(c: dynamic, x: dynamic) -> dynamic
{
  putchar(c);
  write(x);
}

func chkmax(x: dynamic, y: dynamic) -> dynamic
{
   ((x > y)) ? cpp_assign(x, "=", x) : cpp_assign(x, "=", y);
}

func chkmin(x: dynamic, y: dynamic) -> dynamic
{
   ((x < y)) ? cpp_assign(x, "=", x) : cpp_assign(x, "=", y);
}

func max(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  return  ((x > y)) ? ( ((x > z)) ? x : z) : ( ((y > z)) ? y : z);
}

func file(str: dynamic) -> dynamic
{
  freopen(((str + ".in")).c_str(), "r", stdin);
  freopen(((str + ".out")).c_str(), "w", stdout);
}

class Vector
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Vector(x: dynamic = 0, y: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func Vary(argument_0: dynamic) -> dynamic
  {
      return Vector(x, (-y));
    }
  func operator_less(rhs: dynamic) -> dynamic
  {
      return  ((x == rhs.x)) ? (y < rhs.y) : (x < rhs.x);
    }
  func operator_subtract(rhs: dynamic) -> dynamic
  {
      return Vector((x - rhs.x), (y - rhs.y));
    }
  func operator_add(rhs: dynamic) -> dynamic
  {
      return Vector((x + rhs.x), (y + rhs.y));
    }
  func operator_multiply(rhs: dynamic) -> dynamic
  {
      return Vector((x * rhs), (y * rhs));
    }
  func operator_divide(rhs: dynamic) -> dynamic
  {
      return Vector((x / rhs), (y / rhs));
    }
  func operator_multiply(rhs: dynamic) -> dynamic
  {
      return Vector(((x * rhs.x) - (y * rhs.y)), ((x * rhs.y) + (y * rhs.x)));
    }
}

var N: dynamic = (1e6 + 100);

var x: dynamic = cpp_array(N);

var y: dynamic = cpp_array(N);

var w: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var v: dynamic = cpp_array(N);

var in_cpp: dynamic = cpp_array((N * 2));

var e: dynamic = cpp_array(N);

var res: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  read(m);
  {
    var i: dynamic = (1);
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
    var i: dynamic = (1);
    while ((i <= (n)))
    {
      {
        var it: dynamic = (0);
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
        var it: dynamic = (0);
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
        var it: dynamic = (0);
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
    var i: dynamic = (1);
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
    var it: dynamic = (0);
    while ((it < (int_cpp(res.size()))))
    {
      writeln(res[it], cpp_char(" "));
      it += 1;
    }
  }
  puts("");
  {
    var i: dynamic = (1);
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
