// Translated from solution.cpp.

class machine
{
  var n: dynamic = cpp_uninitialized();
  var arr: dynamic = cpp_uninitialized();
  var mn: dynamic = cpp_uninitialized();
  var mx: dynamic = cpp_uninitialized();
  var cur: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_uninitialized();
  func machine(n: dynamic) -> dynamic
  {
      self->n = n;
      sz = 0;
      arr.resize(101);
      mn = 101;
      mx = 0;
      cur = -1;
    }
  func getInput() -> dynamic
  {
      var tmp: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          read(tmp);
          mx = max(mx, tmp);
          mn = min(mn, tmp);
          arr[tmp] += 1;
          i += 1;
        }
      }
    }
  func getBegin() -> dynamic
  {
      sz = 0;
      {
        var i: dynamic = mn;
        while ((i <= mx))
        {
          if (arr[i])
          {
            mn = i;
            return i;
          }
          i += 1;
        }
      }
      return -1;
    }
  func getNext() -> dynamic
  {
      {
        var i: dynamic = cur;
        while ((i <= mx))
        {
          if (arr[i])
          {
            return i;
          }
          i += 1;
        }
      }
      return -1;
    }
  func use(i: dynamic) -> dynamic
  {
      sz += 1;
      cur = max(i, sz);
      if ((arr[i] > 0))
      {
        arr[i] -= 1;
        return 1;
      }
      return -1;
    }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var arr: dynamic = cpp_array(n);
  st.getInput();
  var buf: dynamic = cpp_uninitialized();
  var np: dynamic = 0;
  while (true)
  {
    buf = st.getBegin();
    if ((buf == -1))
    {
      break;
    }
    buf = st.use(buf);
    if ((buf == -1))
    {
      break;
    }
    while (true)
    {
      buf = st.getNext();
      if ((buf == -1))
      {
        break;
      }
      buf = st.use(buf);
      if ((buf == -1))
      {
        break;
      }
    }
    np += 1;
  }
  write(np, "\n");
}
