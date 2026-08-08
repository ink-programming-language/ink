// Translated from solution.cpp.

func isSubSequence(s: dynamic, s1: dynamic, m: dynamic, n: dynamic)
{
  var j = 0;
  {
    var i = 0;
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

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  t = 1;
  while (cpp_update(t, "--"))
  {
    var s1 = cpp_array(1000);
    var s = "heidi";
    read(s1);
    var m = strlen(s);
    var n = strlen(s1);
    if (isSubSequence(s, s1, m, n)) (cout << "YES") else (cout << "NO");
  }
  write("Time taken : ", (cpp_cast(clock()) / CLOCKS_PER_SEC), " secs", "\n");
  return 0;
}
