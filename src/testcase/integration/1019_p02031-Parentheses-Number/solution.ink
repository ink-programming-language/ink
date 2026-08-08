// Translated from solution.cpp.

var MAX_N = 100000;

var qs = cpp_array(MAX_N);

var s = cpp_array(((MAX_N * 2) + 4));

var st: dynamic;

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      var pi: dynamic;
      scanf("%d", (&pi));
      pi -= 1;
      qs[pi] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    var j = 0;
    var k = 0;
    while (((i < n) || (j < n)))
    {
      while (((!st.empty()) && (st.top() == j)))
      {
        s[cpp_update(k, "++")] = cpp_char(")");
        j += 1;
        st.pop();
      }
      if ((i >= n))
      {
        if ((j < n))
        {
          puts(":(");
          return 0;
        }
        break;
      }
      st.push(qs[i]);
      s[cpp_update(k, "++")] = cpp_char("(");
      i += 1;
    }
  }
  puts(s);
  return 0;
}
