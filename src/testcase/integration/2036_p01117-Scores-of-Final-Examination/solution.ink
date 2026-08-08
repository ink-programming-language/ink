// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  while ((n != 0))
  {
    var sub = cpp_construct(n, 0);
    {
      var j = 0;
      while ((j < m))
      {
        {
          var i = 0;
          while ((i < n))
          {
            var a: dynamic;
            read(a);
            sub.at(i) += a;
            i += 1;
          }
        }
        j += 1;
      }
    }
    sort(sub.begin(), sub.end());
    write(sub.at((n - 1)), "\n");
    read(n, m);
  }
}
