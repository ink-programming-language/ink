// Translated from solution.cpp.

var mod = 1000000007;

var inf = 1001001001;

var n: dynamic;

var a: dynamic;

var b: dynamic;

var c: dynamic;

var ans: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  read(n, a, b, c);
  if ((n < min(a, b)))
  {
    write(0);
    return 0;
  }
  if (((a > (b - c)) && ((n - b) >= 0)))
  {
    ans = (((((n - b)) / ((b - c))) + 1) + ((((((n - b)) % ((b - c))) + c)) / a));
  } else
  {
    ans = (n / a);
  }
  write(ans);
  return 0;
}
