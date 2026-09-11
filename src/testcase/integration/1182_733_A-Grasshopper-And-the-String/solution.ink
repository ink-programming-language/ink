// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(s);
  var n: dynamic = s.size();
  var ans: dynamic = (n + 1);
  var low: dynamic = 1;
  var high: dynamic = (n + 1);
  while ((low <= high))
  {
    var mid: dynamic = (((low + high)) / 2);
    var last: dynamic = -1;
    var val: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if (((((((s[i] == cpp_char("A")) || (s[i] == cpp_char("E"))) || (s[i] == cpp_char("I"))) || (s[i] == cpp_char("O"))) || (s[i] == cpp_char("U"))) || (s[i] == cpp_char("Y"))))
        {
          val = max(val, (i - last));
          last = i;
        }
        i += 1;
      }
    }
    val = max(val, (n - last));
    if ((val <= mid))
    {
      ans = mid;
      high = (mid - 1);
    } else
    {
      low = (mid + 1);
    }
  }
  write(ans, "\n");
  return 0;
}
