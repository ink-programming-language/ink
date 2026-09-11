// Translated from solution.cpp.

func main() -> dynamic
{
  var count: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(b);
  {
    var i: dynamic = 0;
    while ((i < b))
    {
      var tmp: dynamic = cpp_uninitialized();
      read(tmp);
      count[tmp] += 1;
      i += 1;
    }
  }
  read(r);
  {
    var i: dynamic = 0;
    while ((i < r))
    {
      var tmp: dynamic = cpp_uninitialized();
      read(tmp);
      count[tmp] -= 1;
      i += 1;
    }
  }
  var max: dynamic = 0;
  for (var x: dynamic in count)
  {
    max = max(max, x.second);
  }
  write(max, "\n");
}
