// Translated from solution.cpp.

func err(it: dynamic)
{
}

func err(it: dynamic, a: dynamic, args: dynamic...)
{
  write((*it), " = ", a, "\n");
  err(cpp_update(it, "++"), cpp_expand(args));
}

var N = 300010;

var mod = (1e9 + 7);

var mod2 = (1e9 + 9);

var mod3 = 998244353;

var sq = 450;

var base = 727;

var lg = 25;

var inf = (1e18 + 10);

var n: dynamic;

var m: dynamic;

var x: dynamic;

var y: dynamic;

var w: dynamic;

var z: dynamic;

var t: dynamic;

var k: dynamic;

var ans: dynamic;

var a = cpp_array(N);

var s: dynamic;

var f = cpp_array(N);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  x = 0;
  ans = 1;
  y = 1;
  while ((y < n))
  {
    y += 1;
    ans += (x + 4);
    x += 4;
  }
  write(ans);
  return 0;
}
