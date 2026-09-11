// Translated from solution.cpp.

func read(x: dynamic) -> dynamic
{
  var c: dynamic = getchar();
  var f: dynamic = 0;
  x = 0;
  while ((!isdigit(c)))
  {
    f |= (c == cpp_char("-"));
    c = getchar();
  }
  while (isdigit(c))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
    c = getchar();
  }
  if (f)
  {
    x = (-x);
  }
  return x;
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    write((-x));
  } else
  {
    if ((x > 9))
    {
      write((x / 10));
    }
    putchar((cpp_char("0") + (x % 10)));
  }
}

var int_cpp: dynamic = dynamic;

var mod: dynamic = (1e9 + 7);

var N: dynamic = (2e5 + 5);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

func doit() -> dynamic
{
  var ans: dynamic = 0;
  var sum: dynamic = cpp_uninitialized();
  var sf: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      s[i] = (s[(i - 1)] + a[i]);
      i += 1;
    }
  }
  sf[0] = cpp_assign(sum, "=", 1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var f: dynamic = sum;
      (cpp_assign(sum, "+=", ((mod - sf[s[(i - 1)]]) + f))) %= mod;
      sf.erase(s[(i - 1)]);
      (cpp_assign(sf[(s[i] - a[i])], "+=", f)) %= mod;
      i += 1;
    }
  }
  for (var x: dynamic in sf)
  {
    (cpp_assign(ans, "+=", x.second)) %= mod;
  }
  write(ans);
  puts("");
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    doit();
  }
}
