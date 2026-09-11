// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  while ((n != 0))
  {
    var sub: dynamic = cpp_construct(n, 0);
    {
      var j: dynamic = 0;
      while ((j < m))
      {
        {
          var i: dynamic = 0;
          while ((i < n))
          {
            var a: dynamic = cpp_uninitialized();
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
