// Translated from solution.cpp.

func toString(n: dynamic) -> dynamic
{
  var os: dynamic = cpp_uninitialized();
  (os << n);
  return os.str();
}

func main() -> dynamic
{
  var Q: dynamic = cpp_uninitialized();
  var N: dynamic = cpp_uninitialized();
  read(Q);
  while (cpp_update(Q, "--"))
  {
    read(N);
    var i: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((N.size() != 1))
      {
        var ans: dynamic = -1;
        {
          var j: dynamic = 1;
          while ((j < N.size()))
          {
            var a: dynamic = N.substr(0, j);
            var b: dynamic = N.substr(j);
            ans = max(ans, (atoi(a.c_str()) * atoi(b.c_str())));
            j += 1;
          }
        }
        N = toString(ans);
        i += 1;
      }
    }
    write(i, "\n");
  }
  return 0;
}
