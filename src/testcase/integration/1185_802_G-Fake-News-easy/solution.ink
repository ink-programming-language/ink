// Translated from solution.cpp.

func isSubSequence(s: dynamic, s1: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  var j: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < n) && (j < m)))
    {
      if ((s[j] == s1[i]))
      {
        j += 1;
      }
      i += 1;
    }
  }
  return ((j == m));
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  t = 1;
  while (cpp_update(t, "--"))
  {
    var s1: dynamic = cpp_array(1000);
    var s: dynamic = "heidi";
    read(s1);
    var m: dynamic = strlen(s);
    var n: dynamic = strlen(s1);
     (isSubSequence(s, s1, m, n)) ? (cout << "YES") : (cout << "NO");
  }
  write("Time taken : ", (cpp_cast(clock()) / CLOCKS_PER_SEC), " secs", "\n");
  return 0;
}
