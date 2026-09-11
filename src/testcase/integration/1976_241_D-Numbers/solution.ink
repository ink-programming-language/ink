// Translated from solution.cpp.

func read() -> dynamic
{
  var s: dynamic = 0;
  var f: dynamic = 0;
  var ch: dynamic = getchar();
  while ((!isdigit(ch)))
  {
    f |= ((ch == cpp_char("-")));
    ch = getchar();
  }
  while (isdigit(ch))
  {
    s = ((((s << 1)) + ((s << 3))) + ((ch ^ 48)));
    ch = getchar();
  }
  return  (f) ? (-s) : s;
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var check: dynamic = cpp_uninitialized();

var MAX: dynamic = 26;

var a: dynamic = cpp_array(MAX);

var b: dynamic = cpp_array(MAX);

var pos: dynamic = cpp_array(MAX);

func pw(x: dynamic) -> dynamic
{
  return  ((x < 10)) ? 10 : 100;
}

func dfs(k: dynamic, x: dynamic, y: dynamic, num: dynamic) -> dynamic
{
  if (cpp_binary(cpp_binary((x == 0), "and", (y == 0)), "and", num))
  {
    puts("Yes");
    printf("%lld\n", num);
    {
      var i: dynamic = 1;
      while ((i <= num))
      {
        printf("%lld ", b[i]);
        i += 1;
      }
    }
    exit(0);
  }
  if ((k > m))
  {
    return;
  }
  dfs((k + 1), x, y, num);
  b[(num + 1)] = pos[a[k]];
  dfs((k + 1), (x ^ a[k]), ((((y * pw(a[k])) + a[k])) % p), (num + 1));
}

func main() -> dynamic
{
  n = read();
  p = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = read();
      if ((x < MAX))
      {
        a[cpp_update(m, "++")] = x;
        pos[x] = i;
      }
      i += 1;
    }
  }
  dfs(1, 0, 0, 0);
  puts("No");
  return 0;
}
