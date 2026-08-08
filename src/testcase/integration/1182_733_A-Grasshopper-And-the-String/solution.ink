// Translated from solution.cpp.

var mod = 1000000007;

var s: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(s);
  var n = s.size();
  var ans = (n + 1);
  var low = 1;
  var high = (n + 1);
  while ((low <= high))
  {
    var mid = (((low + high)) / 2);
    var last = -1;
    var val = 0;
    {
      var i = 0;
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
