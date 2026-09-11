// Translated from solution.cpp.

func err(it: dynamic) -> dynamic
{
}

func err(it: dynamic, a: dynamic, args: dynamic...) -> dynamic
{
  write((*it), " = ", a, "\n");
  err(cpp_update(it, "++"), cpp_expand(args));
}

var N: dynamic = 300010;

var mod: dynamic = (1e9 + 7);

var mod2: dynamic = (1e9 + 9);

var mod3: dynamic = 998244353;

var sq: dynamic = 450;

var base: dynamic = 727;

var lg: dynamic = 25;

var inf: dynamic = (1e18 + 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var s: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

func main() -> dynamic
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
