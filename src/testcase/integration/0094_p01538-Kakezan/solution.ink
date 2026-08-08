// Translated from solution.cpp.

func toString(n: dynamic)
{
  var os: dynamic;
  (os << n);
  return os.str();
}

func main()
{
  var Q: dynamic;
  var N: dynamic;
  read(Q);
  while (cpp_update(Q, "--"))
  {
    read(N);
    var i: dynamic;
    {
      i = 0;
      while ((N.size() != 1))
      {
        var ans = -1;
        {
          var j = 1;
          while ((j < N.size()))
          {
            var a = N.substr(0, j);
            var b = N.substr(j);
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
