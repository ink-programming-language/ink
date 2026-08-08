// Translated from solution.cpp.

var N: dynamic;

var W: dynamic;

var B: dynamic;

var P: dynamic;

var bus = cpp_array(10001);

var win = true;

var ans: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  if (fopen("cf703c.in", "r"))
  {
    freopen("cf703c.in", "r", stdin);
    freopen("cf703c.out", "w", stdout);
  }
  read(N, W, B, P);
  {
    var i = 0;
    while ((i < N))
    {
      var a: dynamic;
      var b: dynamic;
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
