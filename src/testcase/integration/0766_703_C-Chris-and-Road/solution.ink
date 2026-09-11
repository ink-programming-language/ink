// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var P: dynamic = cpp_uninitialized();

var bus: dynamic = cpp_array(10001);

var win: dynamic = true;

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  if (fopen("cf703c.in", "r"))
  {
    freopen("cf703c.in", "r", stdin);
    freopen("cf703c.out", "w", stdout);
  }
  read(N, W, B, P);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      bus[i].first = (a * 1.0);
      bus[i].second = (b * 1.0);
      if (((bus[i].first * P) < (bus[i].second * B)))
      {
        win = false;
      }
      ans = max(ans, (max((bus[i].first / B), (bus[i].second / P)) + (((W - bus[i].second)) / P)));
      i += 1;
    }
  }
  if (win)
  {
    write(fixed, setprecision(20), (W / P), cpp_char("\n"));
    return 0;
  } else
  {
    write(fixed, setprecision(20), ans, cpp_char("\n"));
    return 0;
  }
}
