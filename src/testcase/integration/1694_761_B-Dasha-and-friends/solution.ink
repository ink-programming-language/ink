// Translated from solution.cpp.

var a = cpp_array(100);

var b = cpp_array(100);

func main()
{
  var n: dynamic;
  var L: dynamic;
  read(n, L);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  var p: dynamic;
  var q: dynamic;
  {
    var i = 1;
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
    var i = 0;
    while ((i < n))
    {
      var dum: dynamic;
      {
        var j = 0;
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
