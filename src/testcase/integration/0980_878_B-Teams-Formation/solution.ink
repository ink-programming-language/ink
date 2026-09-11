// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&k), (&b));
  var st: dynamic = cpp_uninitialized();
  var ans: dynamic = (cpp_cast(n) * b);
  {
    var i: dynamic = 0;
    var a: dynamic = cpp_uninitialized();
    while ((i < n))
    {
      scanf("%d", (&a));
      if ((st.size() && (st.top().first == a)))
      {
        st.top().second += 1;
        if ((st.top().second == k))
        {
          ans -= (cpp_cast(k) * b);
          st.pop();
        }
      } else
      {
        st.push([a, 1]);
      }
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  while (st.size())
  {
    while (st.top().second)
    {
      q.push_front(st.top().first);
      st.top().second -= 1;
    }
    st.pop();
  }
  while (q.size())
  {
    var c: dynamic = 0;
    var x: dynamic = q.front();
    while ((q.size() && (q.front() == x)))
    {
      c += 1;
      q.pop_front();
    }
    while ((q.size() && (q.back() == x)))
    {
      c += 1;
      q.pop_back();
    }
    if ((!q.size()))
    {
      var am: dynamic = (cpp_cast(b) * c);
      if (((am % k) == 0))
      {
        return cpp_comma(puts("0"), 0);
      }
      ans -= (((am / k)) * k);
    } else if (((c % k) == 0))
    {
      ans -= (cpp_cast(c) * ((b - 1)));
    } else
    {
      ans -= ((((c / k)) * k) * cpp_cast(((b - 1))));
      break;
    }
  }
  write(ans, "\n");
  return 0;
}
