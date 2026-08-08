// Translated from solution.cpp.

class machine
{
  var n: dynamic;
  var arr: dynamic;
  var mn: dynamic;
  var mx: dynamic;
  var cur: dynamic;
  var sz: dynamic;
  func machine(n: dynamic)
  {
      this->n = n;
      sz = 0;
      arr.resize(101);
      mn = 101;
      mx = 0;
      cur = -1;
    }
  func getInput()
  {
      var tmp: dynamic;
      {
        var i = 0;
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
  func getBegin()
  {
      sz = 0;
      {
        var i = mn;
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
  func getNext()
  {
      {
        var i = cur;
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
  func use(i: dynamic)
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

func main()
{
  var n: dynamic;
  read(n);
  var arr = cpp_array(n);
  st.getInput();
  var buf: dynamic;
  var np = 0;
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
