// Translated from solution.cpp.

func seqs(s: dynamic, v: dynamic)
{
  var sst: dynamic;
  var ret: dynamic;
  {
    var i = 0;
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

func count(s: dynamic, t: dynamic)
{
  var ret = [0, 0];
  {
    var i = 0;
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

func main(argc: dynamic, argv: dynamic)
{
  var str: dynamic;
  var n: dynamic;
  read(str, n);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var ret = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var w = seqs(str, v[i]);
      {
        var j = 0;
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
