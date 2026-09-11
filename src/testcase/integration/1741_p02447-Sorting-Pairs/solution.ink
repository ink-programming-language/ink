// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var A: dynamic = cpp_uninitialized();
  A.reserve(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      A.push_back(make_pair(x, y));
      i += 1;
    }
  }
  sort(A.begin(), A.end());
  {
    var it: dynamic = A.begin();
    while ((it != A.end()))
    {
      write(it->first, cpp_char(" "), it->second, "\n");
      it += 1;
    }
  }
}
