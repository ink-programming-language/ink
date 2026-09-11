// Translated from solution.cpp.

var t: dynamic = cpp_array(5005);

var n: dynamic = cpp_uninitialized();

var used: dynamic = cpp_array(5005);

var ret: dynamic = cpp_array(5005);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(t[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      memset(used, 0, cpp_sizeof(used));
      used[t[i]] += 1;
      var mx: dynamic = t[i];
      ret[t[i]] += 1;
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          used[t[j]] += 1;
          if (((used[t[j]] > used[mx]) || (((used[t[j]] == used[mx]) && (t[j] < mx)))))
          {
            mx = t[j];
          }
          ret[mx] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(ret[i], " ");
      i += 1;
    }
  }
  return 0;
}
