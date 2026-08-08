// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var b: dynamic;
  scanf("%d%d%d", (&n), (&k), (&b));
  var st: dynamic;
  var ans = (cpp_cast(n) * b);
  {
    var i = 0;
    var a: dynamic;
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
  var q: dynamic;
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
    var c = 0;
    var x = q.front();
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
      var am = (cpp_cast(b) * c);
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
