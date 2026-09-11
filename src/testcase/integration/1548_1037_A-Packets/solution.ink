// Translated from solution.cpp.

var N: dynamic = 100001;

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  v.push_back(1);
  {
    var i: dynamic = 1;
    while ((i < 31))
    {
      v.push_back((v[(i - 1)] * 2));
      i += 1;
    }
  }
  var n: dynamic = cpp_uninitialized();
  read(n);
  var pos: dynamic = (upper_bound(v.begin(), v.end(), n) - v.begin());
  write(pos, "\n");
  return 0;
}
