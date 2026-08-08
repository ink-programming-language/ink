// Translated from solution.cpp.

func read()
{
  var s = 0;
  var f = 0;
  var ch = getchar();
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
  return if (f) (-s) else s;
}

var n: dynamic;

var m: dynamic;

var p: dynamic;

var check: dynamic;

var MAX = 26;

var a = cpp_array(MAX);

var b = cpp_array(MAX);

var pos = cpp_array(MAX);

func pw(x: dynamic)
{
  return if ((x < 10)) 10 else 100;
}

func dfs(k: dynamic, x: dynamic, y: dynamic, num: dynamic)
{
  if (cpp_binary(cpp_binary((x == 0), "and", (y == 0)), "and", num))
  {
    puts("Yes");
    printf("%lld\n", num);
    {
      var i = 1;
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

func main()
{
  n = read();
  p = read();
  {
    var i = 1;
    while ((i <= n))
    {
      var x = read();
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
