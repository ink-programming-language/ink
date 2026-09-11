// Translated from solution.cpp.

class node
{
  var index: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_uninitialized();
  func node() -> dynamic
  {
    }
  func node(index: dynamic, num: dynamic) -> dynamic
  {
      index = index;
      num = num;
    }
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.num < b.num);
}

func mylowerbound(vec: dynamic, n: dynamic) -> dynamic
{
  var start: dynamic = 0;
  var end: dynamic = (vec.size() - 1);
  if ((n < vec[0].num))
  {
    return 0;
  }
  if ((n > vec.back().num))
  {
    return vec.size();
  }
  while ((start <= end))
  {
    if ((start == end))
    {
      return start;
    } else if ((start == (end - 1)))
    {
      if ((vec[start].num >= n))
      {
        return start;
      }
      return end;
    } else
    {
      var mid: dynamic = (start + (((end - start)) / 2));
      if ((vec[mid].num >= n))
      {
        end = mid;
      } else
      {
        start = (mid + 1);
      }
    }
  }
  return -1;
}

func main() -> dynamic
{
  var n: dynamic = 0;
  var x1: dynamic = 0;
  var x2: dynamic = 0;
  read(n, x1, x2);
  var vec: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var curnum: dynamic = 0;
      scanf("%d", (&curnum));
      vec.push_back(node(i, curnum));
      i += 1;
    }
  }
  sort(vec.begin(), vec.end(), cmp);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var curNum: dynamic = ((((x1 + i) - 1)) / i);
      var index: dynamic = mylowerbound(vec, curNum);
      var nextIndex: dynamic = (i + index);
      if ((nextIndex < n))
      {
        curNum = vec[nextIndex].num;
        var len: dynamic = ((((x2 + curNum) - 1)) / curNum);
        if ((len <= (n - nextIndex)))
        {
          write("Yes", "\n");
          write(i, " ", len, "\n");
          {
            var j: dynamic = index;
            while ((j < nextIndex))
            {
              write((vec[j].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          {
            var j: dynamic = 0;
            while ((j < len))
            {
              write((vec[(j + nextIndex)].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var curNum: dynamic = ((((x2 + i) - 1)) / i);
      var index: dynamic = mylowerbound(vec, curNum);
      var nextIndex: dynamic = (i + index);
      if ((nextIndex < n))
      {
        curNum = vec[nextIndex].num;
        var len: dynamic = ((((x1 + curNum) - 1)) / curNum);
        if ((len <= (n - nextIndex)))
        {
          write("Yes", "\n");
          write(len, " ", i, "\n");
          {
            var j: dynamic = 0;
            while ((j < len))
            {
              write((vec[(j + nextIndex)].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          {
            var j: dynamic = index;
            while ((j < nextIndex))
            {
              write((vec[j].index + 1), " ");
              j += 1;
            }
          }
          write("\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  write("No", "\n");
  return 0;
}
