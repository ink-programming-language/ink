// Translated from solution.cpp.

func main()
{
  var s = cpp_array(103);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(s[i]);
      i += 1;
    }
  }
  sort((s + 1), ((s + 1) + n));
  {
    var i = 1;
    while ((i <= n))
    {
      write(s[i]);
      i += 1;
    }
  }
}
