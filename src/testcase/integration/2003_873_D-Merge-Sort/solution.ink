// Translated from solution.cpp.

var arr: dynamic = cpp_array(132005);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var init_cnt: dynamic = 1;

var cnt: dynamic = cpp_array((132005 << 1));

func init(l: dynamic, r: dynamic, p: dynamic) -> dynamic
{
  if (((r - l) <= 1))
  {
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  var lch: dynamic = (p << 1);
  var rch: dynamic = ((p << 1) | 1);
  init_cnt += 2;
  cnt[p] += 2;
  init(l, mid, lch);
  init(mid, r, rch);
  cnt[p] += (cnt[lch] + cnt[rch]);
}

var fix: dynamic = cpp_uninitialized();

func divide(l: dynamic, r: dynamic, p: dynamic) -> dynamic
{
  if (((r - l) <= 1))
  {
    return;
  }
  if ((cnt[p] <= fix))
  {
    sort((arr + l), (arr + r));
    fix -= cnt[p];
  } else
  {
    var mid: dynamic = (((l + r)) >> 1);
    var lch: dynamic = (p << 1);
    var rch: dynamic = ((p << 1) | 1);
    if ((cnt[lch] <= fix))
    {
      sort((arr + l), (arr + mid));
      fix -= cnt[lch];
    } else
    {
      divide(l, mid, lch);
    }
    if ((cnt[rch] <= fix))
    {
      sort((arr + mid), (arr + r));
      fix -= cnt[rch];
    } else
    {
      divide(mid, r, rch);
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, k);
  memset(cnt, 0, cpp_sizeof(cnt));
  init(0, n, 1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      arr[i] = (n - i);
      i += 1;
    }
  }
  if ((init_cnt < k))
  {
    write(-1, "\n");
  } else
  {
    fix = (init_cnt - k);
    divide(0, n, 1);
    if ((fix != 0))
    {
      write(-1, "\n");
    } else
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((i != 0))
          {
            write(" ");
          }
          write(arr[i]);
          i += 1;
        }
      }
      write("\n");
    }
  }
  return 0;
}
