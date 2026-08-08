// Translated from solution.cpp.

class node
{
  var data: dynamic;
  var next: dynamic;
}

class node
{
}

var SortedMerge: dynamic;

func MergeSort(headRef: dynamic)
{
  var head = (*headRef);
  var a: dynamic;
  var b: dynamic;
  if ((((head == null)) || ((head->next == null))))
  {
    return;
  }
  FrontBackSplit(head, (&a), (&b));
  MergeSort((&a));
  MergeSort((&b));
  (*headRef) = SortedMerge(a, b);
}

func SortedMerge(a: dynamic, b: dynamic)
{
  var result = null;
  if ((a == null))
  {
    return (b);
  } else if ((b == null))
  {
    return (a);
  }
  if ((a->data <= b->data))
  {
    result = a;
    result->next = SortedMerge(a->next, b);
  } else
  {
    result = b;
    result->next = SortedMerge(a, b->next);
  }
  return (result);
}

func FrontBackSplit(source: dynamic, frontRef: dynamic, backRef: dynamic)
{
  var fast: dynamic;
  var slow: dynamic;
  if (((source == null) || (source->next == null)))
  {
    (*frontRef) = source;
    (*backRef) = null;
  } else
  {
    slow = source;
    fast = source->next;
    while ((fast != null))
    {
      fast = fast->next;
      if ((fast != null))
      {
        slow = slow->next;
        fast = fast->next;
      }
    }
    (*frontRef) = source;
    (*backRef) = slow->next;
    slow->next = null;
  }
}

func printList(node: dynamic)
{
  while ((node != null))
  {
    printf("%d ", node->data);
    node = node->next;
  }
}

func push(head_ref: dynamic, new_data: dynamic)
{
  var new_node = cpp_cast(malloc(cpp_sizeof(dynamic)));
  new_node->data = new_data;
  new_node->next = ((*head_ref));
  ((*head_ref)) = new_node;
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n);
  read(m);
  var a = cpp_new();
  var count = cpp_new();
  var b = cpp_new();
  {
    var i = 0;
    while ((i < n))
    {
      b[i] = null;
      count[i] = 0;
      i += 1;
    }
  }
  var d = 0;
  {
    var i = 0;
    while ((i < m))
    {
      read(a[i]);
      if ((i > 0))
      {
        d = (d + abs((a[i] - a[(i - 1)])));
      }
      i += 1;
    }
  }
  if ((m == 1))
  {
    printf("%d\n", 0);
    return 0;
  }
  {
    var i = 0;
    while ((i < m))
    {
      if ((i > 0))
      {
        if ((a[(i - 1)] != a[i]))
        {
          push((&b[(a[i] - 1)]), a[(i - 1)]);
          count[(a[i] - 1)] += 1;
        }
      }
      if ((i < (m - 1)))
      {
        if ((a[(i + 1)] != a[i]))
        {
          push((&b[(a[i] - 1)]), a[(i + 1)]);
          count[(a[i] - 1)] += 1;
        }
      }
      i += 1;
    }
  }
  var w = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((count[i] != 0))
      {
        MergeSort((&b[i]));
        var temp = b[i];
        {
          var j = 0;
          while ((j < ((((count[i] + 1)) / 2) - 1)))
          {
            temp = temp->next;
            j += 1;
          }
        }
        var median = temp->data;
        var w1 = 0;
        var w2 = 0;
        temp = b[i];
        while ((temp != null))
        {
          w1 += abs(((i + 1) - (temp->data)));
          w2 += abs((median - (temp->data)));
          temp = temp->next;
        }
        if (((w1 - w2) > w))
        {
          w = (w1 - w2);
        }
      }
      i += 1;
    }
  }
  write((d - w), "\n");
}
