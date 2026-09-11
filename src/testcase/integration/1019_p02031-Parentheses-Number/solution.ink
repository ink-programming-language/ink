// Translated from solution.cpp.

var MAX_N: dynamic = 100000;

var qs: dynamic = cpp_array(MAX_N);

var s: dynamic = cpp_array(((MAX_N * 2) + 4));

var st: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var pi: dynamic = cpp_uninitialized();
      scanf("%d", (&pi));
      pi -= 1;
      qs[pi] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    var j: dynamic = 0;
    var k: dynamic = 0;
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
