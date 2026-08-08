// Translated from solution.cpp.

var t = cpp_array(5005);

var n: dynamic;

var used = cpp_array(5005);

var ret = cpp_array(5005);

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(t[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      memset(used, 0, cpp_sizeof(used));
      used[t[i]] += 1;
      var mx = t[i];
      ret[t[i]] += 1;
      {
        var j = (i + 1);
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
    var i = 1;
    while ((i <= n))
    {
      write(ret[i], " ");
      i += 1;
    }
  }
  return 0;
}
