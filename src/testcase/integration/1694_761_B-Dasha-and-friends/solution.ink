// Translated from solution.cpp.

var a: dynamic = cpp_array(100);

var b: dynamic = cpp_array(100);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_uninitialized();
  read(n, L);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      p.push_back((a[i] - a[(i - 1)]));
      q.push_back((b[i] - b[(i - 1)]));
      i += 1;
    }
  }
  p.push_back(((L - a[(n - 1)]) + a[0]));
  q.push_back(((L - b[(n - 1)]) + b[0]));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var dum: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          dum.push_back(q[(((j + i)) % n)]);
          j += 1;
        }
      }
      if ((dum == p))
      {
        write("YES", "\n");
        exit(0);
      }
      i += 1;
    }
  }
  write("NO", "\n");
  return 0;
}
