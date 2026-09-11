// Translated from solution.cpp.

func seqs(s: dynamic, v: dynamic) -> dynamic
{
  var sst: dynamic = cpp_uninitialized();
  var ret: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((sst.get() == null))
      {
        if (((s[i] == v[0]) || (s[i] == v[1])))
        {
          sst.reset(cpp_new());
          ((*sst.get()) << s[i]);
        }
      } else
      {
        if (((s[i] == v[0]) || (s[i] == v[1])))
        {
          ((*sst.get()) << s[i]);
        } else
        {
          ret.push_back(sst->str());
          sst.release();
        }
      }
      i += 1;
    }
  }
  if ((sst.get() != null))
  {
    ret.push_back(sst->str());
  }
  return ret;
}

func count(s: dynamic, t: dynamic) -> dynamic
{
  var ret: dynamic = [0, 0];
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((s[i] == t[0]))
      {
        ret[0] += 1;
      } else
      {
        ret[1] += 1;
      }
      i += 1;
    }
  }
  return min(ret[0], ret[1]);
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  var str: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(str, n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var w: dynamic = seqs(str, v[i]);
      {
        var j: dynamic = 0;
        while ((j < w.size()))
        {
          ret += count(w[j], v[i]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ret, "\n");
  return 0;
}
