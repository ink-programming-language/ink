// Translated from solution.cpp.

var MX: dynamic = (1 << 10);

var n: dynamic = cpp_uninitialized();

var bitcnt: dynamic = cpp_array((MX * 10));

var mp: dynamic = cpp_array((MX * 10));

var ass: dynamic = cpp_array((MX * 10));

var acnt: dynamic = cpp_uninitialized();

var ret: dynamic = cpp_array(MX);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i < ((1 << 13))))
    {
      bitcnt[i] = (1 + bitcnt[(i - ((i & (-i))))]);
      if ((bitcnt[i] == 6))
      {
        ass[cpp_update(acnt, "++")] = i;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      mp[ass[i]] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 13))
    {
      var cnt: dynamic = 0;
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((((((ass[j] >> i)) & 1)) == 0))
          {
            cnt += 1;
          }
          j += 1;
        }
      }
      if ((!cnt))
      {
        i += 1;
        continue;
      }
      write("? ", cnt, " ");
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((((((ass[j] >> i)) & 1)) == 0))
          {
            write(j, " ");
          }
          j += 1;
        }
      }
      write("\n");
      read(ret[i]);
      i += 1;
    }
  }
  write("! ");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var Ans: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < 13))
        {
          if ((((ass[i] >> j)) & 1))
          {
            Ans |= ret[j];
          }
          j += 1;
        }
      }
      write(Ans, " ");
      i += 1;
    }
  }
  write("\n");
}
